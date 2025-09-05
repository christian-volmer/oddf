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

#pragma once

#include <cstddef>

namespace oddf::utility {

// Copies an integer from one buffer to another. The number in the source
// buffer is assumed to have the specified word width and signedness. For
// negative numbers, the two's complement representation is assumed. Bits
// beyond the source word width will be ignored and be replaced by correct
// padding or sign-extension in the destination buffer. If the buffer is too
// small to contain the *actual* source value, the function will throw with
// `ExceptionCode::Overflow`.
void IntegerCopy(void *dest, size_t destSize, void const *src, size_t srcSize, size_t srcWordWidth, bool signedInteger);

// Checks if the buffer represents an integer of given word width and
// signedness with correct padding or sign-extension up to the full size
// of the buffer. Two's complement representation is assumed for negative
// numbers.
bool IntegerCheckIntegrity(void const *buffer, size_t bufferSize, size_t wordWidth, bool signedInteger) noexcept;

// Performs the integrity check according to function
// `IntegerCheckIntegrity()`. If the check fails, the representation will be
// corrected by doing a modulo operation in accordance  with the given word
// width and signedness. This behaviour is also known as 'wrap-around'. Returns
// `true` if the representation was correct in the first place and `false` if
// corrections had to be applied.
bool IntegerFixIntegrity(void *buffer, size_t bufferSize, size_t wordWidth, bool signedInteger);

} // namespace oddf::utility
