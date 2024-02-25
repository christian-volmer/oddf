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

	Select() returns a sequence of elements of a bus based starting at
	the index given by its input node. Implementation for 'dynfix'.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

class select_block_dynfix : public BlockBase {

private:

	struct Input {

		InputPin<dynfix> input;
		int align;

		Input(BlockBase *block, node<dynfix> const &inputNode, int align) :
			input(block, inputNode),
			align(align)
		{
			assert(align >= 0);
		}

	};

	int length;
	int inputWidth;
	int stride;
	InputPin<dynfix> indexInput;
	std::vector<std::unique_ptr<Input>> inputs;
	std::list<OutputPin<dynfix>> outputs;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;

		blocks.insert(indexInput.GetDrivingBlock());

		for (auto &input : inputs)
			blocks.insert(input->input.GetDrivingBlock());

		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		int index = indexInput.GetValue().data[0] * stride;

		if ((index < 0) || (index + length > inputWidth))
			throw design_error(GetFullName() + ": 'Index' input is out of range. 'Index' = " + std::to_string(index) + ", valid range = [0; " + std::to_string(inputWidth - length) + "].");

		for (auto &output : outputs) {

			inputs[index]->input.GetValue().CopyShiftLeft(output.value, inputs[index]->align);
			++index;
		}
	}

	std::string GetInputPinName(int index) const override
	{
		if (index == 0)
			return "Index";
		else if (index >= 1 && index <= (int)inputs.size())
			return "In" + std::to_string(index - 1);

		assert(false);
		return "<ERROR>";
	}

	std::string GetOutputPinDescription(int index, int &groupIndex, int &busSize, int &busIndex) const override
	{
		int length = (int)outputs.size();

		if (length > 1 && index >= 0 && index < length) {

			groupIndex = 0;
			busSize = length;
			busIndex = index;
			return "Out";
		}

		return BlockBase::GetOutputPinDescription(index, groupIndex, busSize, busIndex);
	}

public:

	select_block_dynfix(node<dynfix> const &indexNode, bus_access<dynfix> const &input, int length) :
		BlockBase("select"),
		length(length),
		inputWidth(input.width()),
		indexInput(this, indexNode),
		inputs(),
		outputs()
	{
		auto indexTypeDesc = types::GetDescription(indexNode.GetDriver()->value);

		if (indexTypeDesc.GetFraction() > 0)
			throw design_error(GetFullName() + ": type of 'Index' input must have fractional part less than equal to zero. Current type is '" + indexTypeDesc.ToString() + "'.");

		if (indexTypeDesc.GetWordWidth() - indexTypeDesc.GetFraction() > 31)
			throw design_error(GetFullName() + ": maximum representable number 'Index' input must be less than 2^30. Current type is '" + indexTypeDesc.ToString() + "'.");

		stride = 1 << (-indexTypeDesc.GetFraction());

		std::vector<dynfix> inputTypes;
		inputTypes.reserve(inputWidth);
		for (int i = 0; i < inputWidth; ++i)
			inputTypes.push_back(input[i].GetDriver()->value);

		dynfix commonTemplate = dynfix::CommonRepresentation(inputTypes.cbegin(), inputTypes.cend());
		int fraction = commonTemplate.GetFraction();

		for (int i = 0; i < length; ++i)
			outputs.emplace_back(this, commonTemplate);

		for (int i = 0; i < inputWidth; ++i)
			inputs.push_back(std::unique_ptr<Input>(new Input(this, input[i], fraction - input[i].GetType().GetFraction())));
	}

	bus<dynfix> get_output_bus()
	{
		bus<dynfix> output;
		for (auto &driver : outputs)
			output.append(driver.GetNode());

		return output;
	}

};

}
}

namespace blocks {

bus<dynfix> Select(bus_access<dynfix> const &input, node<dynfix> const &index, int length)
{
	return Design::GetCurrent().NewBlock<backend::blocks::select_block_dynfix>(index, input, length).get_output_bus();
}

}
}
