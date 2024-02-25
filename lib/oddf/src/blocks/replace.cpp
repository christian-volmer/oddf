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

	Replace() replaces a sequence of elements in a bus based on the given
	index and data nodes.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class replace_block : public BlockBase {

private:

	InputPin<int> indexInput;
	std::list<InputPin<T>> originalInputs;
	std::list<InputPin<T>> newInputs;
	std::list<OutputPin<T>> outputs;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;

		blocks.insert(indexInput.GetDrivingBlock());

		for (auto &pin : originalInputs)
			blocks.insert(pin.GetDrivingBlock());

		for (auto &pin : newInputs)
			blocks.insert(pin.GetDrivingBlock());

		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		int index = indexInput.GetValue();
		int width = (int)originalInputs.size();
		int newInputsWidth = (int)newInputs.size();

		if ((index < 0) || (index + newInputsWidth > width))
			throw design_error("'index' input is out of range for 'replace' block used in instance '" + GetHierarchyString() + "'.");

		auto originalIt = originalInputs.begin();
		auto originalEnd = originalInputs.end();
		auto outputIt = outputs.begin();
		auto startIt = outputIt;

		int i = 0;

		// Copy values from originalInputs to outputs
		while (originalIt != originalEnd) {

			if (i == index)
				startIt = outputIt;

			outputIt->value = originalIt->GetValue();

			++outputIt;
			++originalIt;
			++i;
		}

		auto newIt = newInputs.begin();
		
		// Replace values in outputs by values from newInputs starting from the given index
		for (i = 0; i < newInputsWidth; ++i) {

			startIt->value = newIt->GetValue();

			++startIt;
			++newIt;
		}
	}

	std::string GetInputPinName(int index) const override
	{
		if (index == 0)
			return "Index";
		else if (index >= 1 && index < (int)GetInputPins().size())
			return "In" + std::to_string(index - 1);

		assert(false);
		return "<ERROR>";
	}

public:

	replace_block(bus_access<T> const &originalItems, node<int> const &indexNode, bus_access<T> const &newItems) :
		BlockBase("replace"),
		indexInput(this, indexNode),
		originalInputs(),
		newInputs(),
		outputs()
	{
		int width = originalItems.width();
		int newInputsWidth = newItems.width();

		for (int i = 1; i <= width; ++i)
			outputs.emplace_back(this, T());

		for (int i = 1; i <= width; ++i)
			originalInputs.emplace_back(this, originalItems(i));

		for (int i = 1; i <= newInputsWidth; ++i)
			newInputs.emplace_back(this, newItems(i));
	}

	bus<T> get_output_bus()
	{
		bus<T> output;
		for (auto &driver : outputs)
			output.append(driver.GetNode());

		return output;
	}

};

}
}

namespace blocks {

#define IMPLEMENT_REPLACE_FUNCTION(_type_) \
	bus<_type_> Replace(bus_access<_type_> const &originalItems, node<int> const &index, bus_access<_type_> const &newItems) \
	{ \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::replace_block<_type_>>(originalItems, index, newItems); \
		return block.get_output_bus(); \
	}


IMPLEMENT_REPLACE_FUNCTION(bool)
IMPLEMENT_REPLACE_FUNCTION(double)
IMPLEMENT_REPLACE_FUNCTION(std::int32_t)
IMPLEMENT_REPLACE_FUNCTION(std::int64_t)

}
}
