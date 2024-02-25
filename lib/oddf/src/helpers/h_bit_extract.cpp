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

	Helper function that extracts bits from the two's complement
	representation of an std::int32_t constant.

*/

#include "../global.h"

#include "h_bit_extract.h"
#include "../blocks/constant.h"

namespace dfx {

bus<bool> BitExtract(std::int32_t constant, int firstBitIndex, int lastBitIndex)
{
	if (firstBitIndex < 0)
		throw design_error("dfx::BitExtract: Parameter firstBitIndex must be non-zeros.");

	if (lastBitIndex < 0)
		throw design_error("dfx::BitExtract: Parameter lastBitIndex must be non-zeros.");

	int maxWidth = sizeof(std::int32_t) * 8;

	if (firstBitIndex >= maxWidth)
		throw design_error("dfx::BitExtract: Parameter firstBitIndex exceeds the actual number of bits of the input data type.");

	if (lastBitIndex >= maxWidth)
		throw design_error("dfx::BitExtract: Parameter lastBitIndex exceeds the actual number of bits of the input data type.");

	int width = std::abs(lastBitIndex - firstBitIndex) + 1;


	std::uint64_t value = (std::uint64_t)constant;

	int position = firstBitIndex;
	int increment = firstBitIndex < lastBitIndex ? 1 : -1;

	std::list<bool> values;
	for (int i = 0; i < width; ++i) {

		values.push_back((value & (1ull << position)) != 0);
		position += increment;
	}

	return blocks::Constant(values.cbegin(), values.cend());
}

}
