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

	BitExtract() extracts bits from the two's complement representation
	of a number.

*/

#include "../global.h"

namespace dfx {

namespace backend {
namespace blocks {

template<typename T> class bit_extract_block : public BlockBase {

private:

	int firstBitIndex;
	int lastBitIndex;
	InputPin<T> valueInput;
	std::list<OutputPin<bool>> outputs;

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t({ valueInput.GetDrivingBlock() });
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		std::uint64_t value = (std::uint64_t)valueInput.GetValue();

		int position = firstBitIndex;
		int increment = firstBitIndex < lastBitIndex ? 1 : -1;

		for (auto &output : outputs) {

			output.value = (value & (1ull << position)) != 0;
			position += increment;
		}
	}

	std::string GetInputPinName(int index) const override
	{
		if (index == 0)
			return "In";

		assert(false);
		return "<ERROR>";
	}


public:

	bit_extract_block(node<T> const &value, int firstBitIndex, int lastBitIndex) :
		BlockBase("bit_extract"),
		firstBitIndex(firstBitIndex),
		lastBitIndex(lastBitIndex),
		valueInput(this, value),
		outputs()
	{
		if (firstBitIndex < 0)
			throw design_error(GetFullName() + ": Parameter firstBitIndex must be non-zeros.");

		if (lastBitIndex < 0)
			throw design_error(GetFullName() + ": Parameter lastBitIndex must be non-zeros.");

		int maxWidth = sizeof(T) * 8;

		if (firstBitIndex >= maxWidth)
			throw design_error(GetFullName() + ": Parameter firstBitIndex exceeds the actual number of bits of the input data type.");

		if (lastBitIndex >= maxWidth)
			throw design_error(GetFullName() + ": Parameter lastBitIndex exceeds the actual number of bits of the input data type.");

		int width = std::abs(lastBitIndex - firstBitIndex) + 1;

		for (int i = 1; i <= width; ++i)
			outputs.emplace_back(this, false);
	}

	bus<bool> get_output_bus()
	{
		bus<bool> output;
		for (auto &driver : outputs)
			output.append(driver.GetNode());

		return output;
	}

};

}
}

namespace blocks {

bus<bool> BitExtract(node<std::int32_t> const &value, int firstBitIndex, int lastBitIndex)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::bit_extract_block<std::int32_t>>(value, firstBitIndex, lastBitIndex);
	return block.get_output_bus();
}

bus<bool> BitExtract(node<std::int64_t> const &value, int firstBitIndex, int lastBitIndex)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::bit_extract_block<std::int64_t>>(value, firstBitIndex, lastBitIndex);
	return block.get_output_bus();
}

}
}
