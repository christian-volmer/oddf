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

    Functions for copying data from one memory location to another
    under the assumption that the data represents a signed or
    unsigned integer value.

*/

#include <oddf/utility/CopyBoolean.h>

#include <oddf/Exception.h>

#include <cstdint>

namespace oddf::utility {

void CopyUnsignedInteger(void *destination, size_t destinationSize, void const *source, size_t sourceSize)
{
	using uchar = unsigned char;

	uchar const *sourceAsUchar = static_cast<uchar const *>(source);
	uchar *destinationAsUchar = static_cast<uchar *>(destination);

	if (destinationSize >= sourceSize) {

		size_t i = 0;
		for (; i < sourceSize; ++i)
			destinationAsUchar[i] = sourceAsUchar[i];

		for (; i < destinationSize; ++i)
			destinationAsUchar[i] = 0;
	}
	else {

		size_t i = 0;
		for (; i < destinationSize; ++i)
			destinationAsUchar[i] = sourceAsUchar[i];

		for (; i < sourceSize; ++i)
			if (sourceAsUchar[i] != 0)
				throw Exception(ExceptionCode::Overflow);
	}
}

void CopySignedInteger(void *destination, size_t destinationSize, void const *source, size_t sourceSize)
{
	using uchar = unsigned char;

	uchar const *sourceAsUchar = static_cast<uchar const *>(source);
	uchar *destinationAsUchar = static_cast<uchar *>(destination);

	if (destinationSize >= sourceSize) {

		size_t i = 0;
		for (; i < sourceSize; ++i)
			destinationAsUchar[i] = sourceAsUchar[i];

		uchar signExtension = (sourceSize > 0) && (sourceAsUchar[sourceSize - 1] >= 0x80) ? 0xff : 0;

		for (; i < destinationSize; ++i)
			destinationAsUchar[i] = signExtension;
	}
	else {

		size_t i = 0;
		for (; i < destinationSize; ++i)
			destinationAsUchar[i] = sourceAsUchar[i];

		uchar signExtension = (destinationSize > 0) && (sourceAsUchar[destinationSize - 1] >= 0x80) ? 0xff : 0;

		for (; i < sourceSize; ++i)
			if (sourceAsUchar[i] != signExtension)
				throw Exception(ExceptionCode::Overflow);
	}
}

} // namespace oddf::utility
