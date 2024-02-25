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

	Decide() provides nested if-then-else functionality as a design
	block.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class decide_block : public BlockBase {

private:

	InputPin<bool> decisionInput;

	struct Path {

		InputPin<T> trueInput;
		InputPin<T> falseInput;
		OutputPin<T> output;

		Path(BlockBase *block, node<T> const &trueNode, node<T> const &falseNode) :
			trueInput(block, trueNode),
			falseInput(block, falseNode),
			output(block, T())
		{
		}
	};

	std::list<Path> paths;
	
	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;

		blocks.insert(decisionInput.GetDrivingBlock());

		for (auto &path : paths) {

			blocks.insert(path.trueInput.GetDrivingBlock());
			blocks.insert(path.falseInput.GetDrivingBlock());
		}

		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	std::string GetInputPinName(int index) const override
	{
		if (index == 0)
			return "Decision";
		else if (index >= 1 && index < (int)GetInputPins().size())
			return ((index - 1) % 2 == 0 ? "True" : "False") + std::to_string((index - 1) / 2);

		assert(false);
		return "<ERROR>";
	}

	void Evaluate() override
	{
		if (decisionInput.GetValue()) {

			for (auto &p : paths)
				p.output.value = p.trueInput.GetValue();
		}
		else {

			for (auto &p : paths)
				p.output.value = p.falseInput.GetValue();
		}
	}

	std::string GetOutputPinDescription(int index, int &groupIndex, int &busSize, int &busIndex) const override
	{
		int length = (int)paths.size();

		if (length > 1 && index >= 0 && index < length) {

			groupIndex = 0;
			busSize = length;
			busIndex = index;
			return "Out";
		}

		return BlockBase::GetOutputPinDescription(index, groupIndex, busSize, busIndex);
	}

public:

	decide_block(node<bool> decisionNode) :
		BlockBase("decide"),
		decisionInput(this, decisionNode),
		paths()
	{
	}

	node<T> add_path(node<T> const &trueNode, node<T> const &falseNode)
	{
		paths.emplace_back(this, trueNode, falseNode);
		return paths.back().output.GetNode();
	}

	bus<T> add_bus(bus_access<T> const &trueInput, bus_access<T> const &falseInput)
	{
		int width = trueInput.width();

		if (falseInput.width() != width)
			throw design_error(GetFullName() + ": Operands of bus operations must have the same width.");

		bus<T> outputBus;
		for (int i = 0; i < width; ++i)
			outputBus.append(add_path(trueInput[i], falseInput[i]));

		return outputBus;
	}
};

}
}

namespace blocks {

#define IMPLEMENT_DECIDE_FUNCTION(_type_) \
	node<_type_> Decide(node<bool> const &decisionNode, node<_type_> const &trueInput, node<_type_> const &falseInput) \
	{ \
		return Design::GetCurrent().NewBlock<backend::blocks::decide_block<_type_>>(decisionNode).add_path(trueInput, falseInput); \
	} \
 \
	bus<_type_> Decide(node<bool> const &decisionNode, bus_access<_type_> const &trueInput, bus_access<_type_> const &falseInput) \
	{ \
		return Design::GetCurrent().NewBlock<backend::blocks::decide_block<_type_>>(decisionNode).add_bus(trueInput, falseInput); \
	} \
 \
	node<_type_> Decide(node<bool> const &decisionNode, node<_type_> const &trueInput, _type_ const &falseInput) \
	{ \
		return Decide(decisionNode, trueInput, Constant(falseInput)); \
	} \
 \
	node<_type_> Decide(node<bool> const &decisionNode, _type_ const &trueInput, node<_type_> const &falseInput) \
	{ \
		return Decide(decisionNode, Constant(trueInput), falseInput); \
	} \
 \
	node<_type_> Decide(node<bool> const &decisionNode, _type_ const &trueInput, _type_ const &falseInput) \
	{ \
		return Decide(decisionNode, Constant(trueInput), Constant(falseInput)); \
	} \
\
	bus<_type_> Decide(node<bool> const &decisionNode, bus_access<_type_> const &trueInput, node<_type_> const &falseInput) \
	{ \
		return Decide(decisionNode, trueInput, bus<_type_>(falseInput, trueInput.width())); \
	} \
 \
	bus<_type_> Decide(node<bool> const &decisionNode, node<_type_> const &trueInput, bus_access<_type_> const &falseInput) \
	{ \
		return Decide(decisionNode, bus<_type_>(trueInput, falseInput.width()), falseInput); \
	} \
 \
	bus<_type_> Decide(node<bool> const &decisionNode, bus_access<_type_> const &trueInput, _type_ const &falseInput) \
	{ \
		return Decide(decisionNode, trueInput, bus<_type_>(Constant(falseInput), trueInput.width())); \
	} \
 \
	bus<_type_> Decide(node<bool> const &decisionNode, _type_ const &trueInput, bus_access<_type_> const &falseInput) \
	{ \
		return Decide(decisionNode, bus<_type_>(Constant(trueInput), falseInput.width()), falseInput); \
	}

IMPLEMENT_DECIDE_FUNCTION(bool)
IMPLEMENT_DECIDE_FUNCTION(double)
IMPLEMENT_DECIDE_FUNCTION(std::int32_t)
IMPLEMENT_DECIDE_FUNCTION(std::int64_t)

}
}
