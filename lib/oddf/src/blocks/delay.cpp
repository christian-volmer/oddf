/*

	ODDF - Open Digital Design Framework
	Copyright Advantest Corporation
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

/*

	Delay() inserts a pipeline delay (flip-flop) into the design.

*/

#include "../global.h"
#include "../simulator_optimisations.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class delay_block : public BlockBase, private IStep {

protected:

	struct Path {
	
		T state;
		InputPin<T> input;
		OutputPin<T> output;

		Path(BlockBase *block, node<T> const &inputNode, T const &initState) :
			state(initState),
			input(block, inputNode),
			output(block, initState)
		{
		}

	};

	std::list<Path> paths;
	bool allTheSameType;

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t();
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		for (auto &p : paths)
			types::Copy(p.output.value, p.state);
	}

	void Step() override
	{
#if defined(DFX_SIMULATOR_DELAY_DIRTY_ON_CHANGE) && (DFX_SIMULATOR_DELAY_DIRTY_ON_CHANGE == 1)

		bool changed = false;

		for (auto &p : paths) {

			if (changed || !types::IsEqual(p.state, p.input.GetValue())) {

				changed = true;
				types::Copy(p.state, p.input.GetValue());
			}
		}

		if (changed)
			SetDirty();

#elif defined(DFX_SIMULATOR_DELAY_DIRTY_ON_CHANGE) && (DFX_SIMULATOR_DELAY_DIRTY_ON_CHANGE == 0)

		for (auto &p : paths)
			types::Copy(p.state, p.input.GetValue());

		SetDirty();

#else

		DFX_SIMULATOR_DELAY_DIRTY_ON_CHANGE must be set to 0 or 1.

#endif
	}

	void AsyncReset() override
	{
		for (auto &p : paths)
			types::Copy(p.state, types::DefaultFrom(p.state));

		SetDirty();
	}

	IStep *GetStep() override
	{
		return this;
	}

	std::string GetInputPinName(int index) const override
	{
		if (index >= 0 && index < (int)paths.size()) {

			if (paths.size() == 1)
				return "In";
			else
				return "In" + std::to_string(index);
		}

		assert(false);
		return "<ERROR>";
	}

	std::string GetOutputPinDescription(int index, int &groupIndex, int &busSize, int &busIndex) const override
	{
		int length = (int)paths.size();

		if (allTheSameType && length > 1 && index >= 0 && index < length) {

			groupIndex = 0;
			busSize = length;
			busIndex = index;
			return "Out";
		}

		return BlockBase::GetOutputPinDescription(index, groupIndex, busSize, busIndex);
	}
	

public:

	delay_block(std::string const &tag) :
		BlockBase("delay", tag),
		paths(),
		allTheSameType(true)
	{
	}

	node<T> add_path(node<T> const &input)
	{
		auto initState = types::DefaultFrom(input.GetDriver()->value);

		if (!paths.empty())
			allTheSameType = allTheSameType && (types::GetDescription(initState) == types::GetDescription(paths.back().state));

		paths.emplace_back(this, input, initState);

		return paths.back().output.GetNode();
	}
};

template<typename T> class enabled_delay_block : public delay_block<T> {

protected:

	InputPin<bool> enableInput;

	void Step() override
	{
		if (enableInput.GetValue())
			delay_block<T>::Step();
	}

	std::string GetInputPinName(int index) const override
	{
		if (index == 0)
			return "Enable";
		else
			return GetInputPinName(index - 1);

		assert(false);
		return "<ERROR>";
	}

public:

	enabled_delay_block(std::string const &tag) :
		delay_block<T>(tag),
		enableInput(this, Design::GetCurrent().customDefaultEnable)
	{
	}
};



}
}

namespace blocks {

#define IMPLEMENT_DELAY_FUNCTION(_type_) \
	node<_type_> Delay(node<_type_> const &input, std::string const &tag) \
	{ \
		auto &block = Design::GetCurrent().hasCustomDefaultEnable \
			? Design::GetCurrent().NewBlock<backend::blocks::enabled_delay_block<_type_>>(tag) \
			: Design::GetCurrent().NewBlock<backend::blocks::delay_block<_type_>>(tag); \
		return block.add_path(input); \
	} \
 \
	bus<_type_> Delay(bus_access<_type_> const &inputBus, std::string const &tag) \
	{ \
		unsigned width = inputBus.width(); \
 \
		if (width == 0) \
			throw design_error("dfx::blocks::Delay: parameter 'inputBus' must have at least one element."); \
 \
		auto &block = Design::GetCurrent().hasCustomDefaultEnable \
			? Design::GetCurrent().NewBlock<backend::blocks::enabled_delay_block<_type_>>(tag) \
			: Design::GetCurrent().NewBlock<backend::blocks::delay_block<_type_>>(tag); \
 \
		bus<_type_> outputBus; \
 \
		for (unsigned i = 1; i <= width; ++i) \
			outputBus.append(block.add_path(inputBus(i))); \
 \
		return outputBus; \
	}

IMPLEMENT_DELAY_FUNCTION(bool)
IMPLEMENT_DELAY_FUNCTION(double)
IMPLEMENT_DELAY_FUNCTION(std::int32_t)
IMPLEMENT_DELAY_FUNCTION(std::int64_t)
IMPLEMENT_DELAY_FUNCTION(dynfix)

}
}
