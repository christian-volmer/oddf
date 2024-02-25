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

	BitCompose() converts a bus of bits to a number. Implementation for
	'dynfix'.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

class bit_compose_block_dynfix : public BlockBase {

private:

	std::list<InputPin<bool>> bitInputs;
	OutputPin<dynfix> output;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;

		for (auto &pin : bitInputs)
			blocks.insert(pin.GetDrivingBlock());

		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		int position = 0;
		for (auto &input : bitInputs) {

			if (input.GetValue())
				output.value.data[position / 32] |= (1 << (position % 32));
			else
				output.value.data[position / 32] &= ~(1 << (position % 32));

			++position;
		}

		// TODO: actually we only have to do the sign extension.
		output.value.OverflowWrapAround();
	}

	std::string GetInputPinName(int index) const override
	{
		if (index >= 0 && index < (int)GetInputPins().size())
			return "Bit" + std::to_string(index);

		assert(false);
		return "<ERROR>";
	}

public:

	bit_compose_block_dynfix(dynfix const &outputTemplate, bus_access<bool> const &bits) :
		BlockBase("bit_compose"),
		bitInputs(),
		output(this, outputTemplate)
	{
		int actualWidth = bits.width();
		int expectedWidth = outputTemplate.GetWordWidth();

		if (actualWidth != expectedWidth)
			throw design_error(GetFullName() + string_printf(": width of the bit bus (%d) is expected to match the number of bits of the result data type (%d).", actualWidth, expectedWidth));

		for (int i = 0; i < actualWidth; ++i)
			bitInputs.emplace_back(this, bits[i]);
	}

	node<dynfix> get_output_node()
	{
		return output.GetNode();
	}

};

}
}

namespace blocks {


template<> node<dynfix> BitCompose(dynfix const &outputTemplate, bus_access<bool> const &bits)
{
	return Design::GetCurrent().NewBlock<backend::blocks::bit_compose_block_dynfix>(outputTemplate, bits).get_output_node();
}

}
}
