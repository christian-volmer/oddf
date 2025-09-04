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

    Function for copying data from one memory location to another under
    the assumption that the data represents a boolean value.

*/

#include <oddf/utility/BooleanSupport.h>

#include <oddf/Exception.h>

#include <algorithm>

namespace oddf::utility {

void BooleanCopy(void *dest, size_t destSize, void const *src, size_t srcSize)
{
	// The following works even if `src` and `dest` overlap.

	using byte = unsigned char;

	auto *srcBytes = static_cast<byte const *>(src);
	auto *destBytes = static_cast<byte *>(dest);

	bool value = std::any_of(srcBytes, srcBytes + srcSize, [](byte b) -> bool { return b; });

	if (value) {

		if (destSize < 1)
			throw Exception(ExceptionCode::Overflow);

		destBytes[0] = 1;
		std::fill(destBytes + 1, destBytes + destSize, byte(0));
	}
	else
		std::fill(destBytes, destBytes + destSize, byte(0));
}

bool BooleanCheckIntegrity(void const *buffer, size_t bufferSize) noexcept
{
	using byte = unsigned char;

	auto *bufferBytes = static_cast<byte const *>(buffer);

	// A zero sized buffer is valid and represents the value `false`.
	if (bufferSize < 1)
		return true;

	// The first byte must be either `0` or `1`.
	if ((bufferBytes[0] & 1) != bufferBytes[0])
		return false;

	// All other bytes must be zero
	if (std::any_of(bufferBytes + 1, bufferBytes + bufferSize, [](byte b) -> bool { return b; }))
		return false;

	return true;
}

bool BooleanFixIntegrity(void *buffer, size_t bufferSize) noexcept
{
	if (BooleanCheckIntegrity(buffer, bufferSize))
		return true;
	else {

		// `BooleanCopy` will fix any errors in the representation.
		BooleanCopy(buffer, bufferSize, buffer, bufferSize);
		return false;
	}
}

} // namespace oddf::utility
