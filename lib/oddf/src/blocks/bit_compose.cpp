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

	BitCompose() converts a bus of bits to a number.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class bit_compose_block : public BlockBase {

private:

	std::list<InputPin<bool>> bitInputs;
	OutputPin<T> output;

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
		std::uint64_t result = 0;
		std::uint64_t value = 1;

		for (auto &pin : bitInputs) {

			if (pin.GetValue())
				result |= value;

			value <<= 1;
		}

		output.value = static_cast<T>(result);
	}

	std::string GetInputPinName(int index) const override
	{
		if (index >= 0 && index < (int)GetInputPins().size())
			return "Bit" + std::to_string(index);

		assert(false);
		return "<ERROR>";
	}

public:

	bit_compose_block(bus_access<bool> const &bits) :
		BlockBase("bit_compose"),
		bitInputs(),
		output(this, T())
	{
		int actualWidth = bits.width();
		int expectedWidth = sizeof(T) * 8;

		if (actualWidth != expectedWidth)
			throw design_error(GetFullName() + string_printf(": width of the bit bus (%d) is expected to match the number of bits of the result data type (%d).", actualWidth, expectedWidth));

		for (int i = 0; i < actualWidth; ++i)
			bitInputs.emplace_back(this, bits[i]);
	}

	node<T> get_output_node()
	{
		return output.GetNode();
	}

};

}
}

namespace blocks {


template<> node<std::int32_t> BitCompose(std::int32_t const &, bus_access<bool> const &bits)
{
	return Design::GetCurrent().NewBlock<backend::blocks::bit_compose_block<std::int32_t>>(bits).get_output_node();
}

template<> node<std::int64_t> BitCompose(std::int64_t const &, bus_access<bool> const &bits)
{
	return Design::GetCurrent().NewBlock<backend::blocks::bit_compose_block<std::int64_t>>(bits).get_output_node();
}

}
}
