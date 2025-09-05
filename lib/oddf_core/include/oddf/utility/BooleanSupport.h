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
    the assumption that the data represents a Boolean value.

*/

#pragma once

#include <cstddef>

namespace oddf::utility {

// Copies a Boolean from one buffer to another. The presence of any non-zero
// byte within the buffer will be considered to represent the value `true`.
// The values `true` and `false` will be represented as `1` and `0`, in the
// first byte of the destination buffer, respectively. Any remaining bytes will
// be cleared.
void BooleanCopy(void *dest, size_t destSize, void const *src, size_t srcSize);

// Returns if the representation in `buffer` contains either `0` or `1` in the
// first byte and the remaining bytes are zero.
bool BooleanCheckIntegrity(void const *buffer, size_t bufferSize) noexcept;

// Checks the Boolean representation in `buffer` in the same way as
// `BooleanCheckIntegrity()`. If necessary the data will be corrected according
// to the rules described for the `BooleanCopy()` function. Returns `true` if
// the representation was correct in the first place and `false` if corrections
// had to be applied.
bool BooleanFixIntegrity(void *buffer, size_t bufferSize) noexcept;

} // namespace oddf::utility
