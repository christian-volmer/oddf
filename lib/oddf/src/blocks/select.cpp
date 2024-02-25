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
	the index given by its input node.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class select_block : public BlockBase {

private:

	int length;
	int inputWidth;
	int stride;
	InputPin<dynfix> indexInput;
	std::vector<std::unique_ptr<InputPin<T>>> inputs;
	std::list<OutputPin<T>> outputs;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;

		blocks.insert(indexInput.GetDrivingBlock());

		for (auto &pin : inputs)
			blocks.insert(pin->GetDrivingBlock());

		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		assert(length == (int)outputs.size());
		assert(inputWidth == (int)inputs.size());

		int index = indexInput.GetValue().data[0] * stride;

		if ((index < 0) || (index > inputWidth - length))
			throw design_error(GetFullName() + ": 'Index' input is out of range. 'Index' = " + std::to_string(index) + ", valid range = [0; " + std::to_string(inputWidth - length) + "].");

		for (auto &output : outputs) {

			output.value = inputs[index]->GetValue();
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

	select_block(node<dynfix> const &indexNode, bus_access<T> const &input, int length) :
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

		if (indexTypeDesc.GetWordWidth() > 31)
			throw design_error(GetFullName() + ": word width of 'Index' input must be at most 31. Current type is '" + indexTypeDesc.ToString() + "'.");

		stride = 1 << (-indexTypeDesc.GetFraction());

		for (int i = 1; i <= length; ++i)
			outputs.emplace_back(this, T());

		for (int i = 1; i <= inputWidth; ++i)
			inputs.push_back(std::unique_ptr<InputPin<T>>(new InputPin<T>(this, input(i))));
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

#define IMPLEMENT_SELECT_FUNCTION(_type_) \
	bus<_type_> Select(bus_access<_type_> const &input, node<dynfix> const &index, int length) \
	{ \
		return Design::GetCurrent().NewBlock<backend::blocks::select_block<_type_>>(index, input, length).get_output_bus(); \
	}


IMPLEMENT_SELECT_FUNCTION(bool)
IMPLEMENT_SELECT_FUNCTION(double)
IMPLEMENT_SELECT_FUNCTION(std::int32_t)
IMPLEMENT_SELECT_FUNCTION(std::int64_t)

}
}
