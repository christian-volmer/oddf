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

#pragma once

namespace dfx {
namespace blocks {

#define DECLARE_BIT_EXTRACT_FUNCTION(_type_) \
	bus<bool> BitExtract(node<_type_> const &value, int firstBitIndex, int lastBitIndex); \
	inline node<bool> BitExtract(node<_type_> const &value, int bitIndex) \
	{ \
		return BitExtract(value, bitIndex, bitIndex).first(); \
	}

DECLARE_BIT_EXTRACT_FUNCTION(std::int32_t)
DECLARE_BIT_EXTRACT_FUNCTION(std::int64_t)
DECLARE_BIT_EXTRACT_FUNCTION(dynfix)

inline bus<bool> BitExtract(node<std::int32_t> const &value)
{
	return BitExtract(value, 0, 31);
}

inline bus<bool> BitExtract(node<std::int64_t> const &value)
{
	return BitExtract(value, 0, 63);
}

inline bus<bool> BitExtract(node<dynfix> const &value)
{
	return BitExtract(value, -value.GetType().GetFraction(), value.GetType().GetWordWidth() - value.GetType().GetFraction() - 1);
}

#undef DECLARE_BIT_EXTRACT_FUNCTION

}
}
