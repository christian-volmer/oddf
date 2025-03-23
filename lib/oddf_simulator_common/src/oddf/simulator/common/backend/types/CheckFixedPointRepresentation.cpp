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

    <no description>

*/

#include <oddf/simulator/common/backend/types/CheckFixedPointRepresentation.h>

namespace oddf::simulator::common::backend::types {

bool CheckFixedPointRepresentation(FixedPoint const &fixedPoint, design::NodeType const &nodeType)
{
	return CheckFixedPointRepresentation(fixedPoint.m_elements, fixedPoint.m_length, nodeType);
}

bool CheckFixedPointRepresentation(FixedPoint::ElementType const *elements, size_t elementCount, design::NodeType const &nodeType)
{
	if (elementCount != FixedPoint::RequiredElementCount(nodeType))
		return false;

	size_t wordWidth = nodeType.GetWordWidth();

	size_t partialWidth = wordWidth % FixedPoint::ElementBitWidth;

	if (partialWidth) {

		// Content of the most significant element
		FixedPoint::ElementType partialContent = elements[wordWidth / FixedPoint::ElementBitWidth];

		// Bitmask for unused bits
		FixedPoint::ElementType mask = FixedPoint::SignedExtension << partialWidth;

		if (nodeType.IsSigned()) {

			bool isNegative = (partialContent >> (partialWidth - 1)) & 1;

			if (isNegative)
				return (partialContent & mask) == mask;
			else
				return (partialContent & mask) == 0;
		}
		else {

			// If the number is unsigned, all unused bits must be zero.
			return (partialContent & mask) == 0;
		}
	}
	else
		return true;
}

} // namespace oddf::simulator::common::backend::types
