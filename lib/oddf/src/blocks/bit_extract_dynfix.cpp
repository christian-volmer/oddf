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
	of a number. Implementation for 'dynfix'.

*/

#include "../global.h"
#include "../generator/properties.h"

namespace dfx {

namespace backend {
namespace blocks {

class bit_extract_block_dynfix : public BlockBase {

private:

	int firstPosition;
	int increment;
	InputPin<dynfix> valueInput;
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
		dynfix const &value = valueInput.GetValue();

		int position = firstPosition;

		for (auto &output : outputs) {

			output.value = (value.data[position / 32] & (1 << (position % 32))) != 0;
			position += increment;
		}
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

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		if (increment > 0) {

			properties.SetInt("FirstBitIndex", firstPosition);
			properties.SetInt("LastBitIndex", firstPosition + (int)outputs.size() - 1);
		}
		else {

			properties.SetInt("FirstBitIndex", firstPosition);
			properties.SetInt("LastBitIndex", firstPosition - (int)outputs.size() + 1);
		}
	}


public:

	bit_extract_block_dynfix(node<dynfix> const &value, int firstBitIndex, int lastBitIndex) :
		BlockBase("bit_extract"),
		valueInput(this, value),
		outputs()
	{
		auto valueTypeDesc = types::GetDescription(value.GetDriver()->value);

		int minIndex = -valueTypeDesc.GetFraction();
		int maxIndex = valueTypeDesc.GetWordWidth() - valueTypeDesc.GetFraction() - 1;

		if (firstBitIndex < minIndex || firstBitIndex > maxIndex)
			throw design_error(GetFullName() + string_printf(": Parameter 'firstBitIndex' (%d) must be in the range [%d %d].", firstBitIndex, minIndex, maxIndex));

		if (lastBitIndex < minIndex || lastBitIndex > maxIndex)
			throw design_error(GetFullName() + string_printf(": Parameter 'lastBitIndex' (%d) must be in the range [%d %d].", lastBitIndex, minIndex, maxIndex));

		firstPosition = firstBitIndex - minIndex;
		increment = firstBitIndex < lastBitIndex ? 1 : -1;

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

bus<bool> BitExtract(node<dynfix> const &value, int firstBitIndex, int lastBitIndex)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::bit_extract_block_dynfix>(value, firstBitIndex, lastBitIndex);
	return block.get_output_bus();
}

}
}
