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

#include <oddf/utility/IntegerSupport.h>

#include <oddf/Exception.h>

#include <algorithm>

namespace oddf::utility {

void IntegerCopy(void *dest, size_t destSize, void const *src, size_t srcSize, size_t srcWordWidth, bool signedInteger)
{
	// The following works even if `src` and `dest` are equal.

	using byte = unsigned char;

	auto *srcBytes = static_cast<byte const *>(src);
	auto *destBytes = static_cast<byte *>(dest);

	// Quick return: a zero word width corresponds to the value zero.
	if (srcWordWidth == 0) {

		std::fill(destBytes, destBytes + destSize, byte(0));
		return;
	}

	// Confirm that the size of the source buffer fits to the specified word width
	size_t usedSize = (srcWordWidth + 7) / 8;

	if (srcSize < usedSize)
		throw Exception(ExceptionCode::InvalidArgument);

	// The number of valid bits in the most significant byte, or 0 if all bits are valid.
	size_t partialBitCount = srcWordWidth % 8;

	// Mask for the extension bits in the most significant byte
	byte extensionMask = partialBitCount == 0 ? 0 : byte(-1) << partialBitCount;

	// Mask for the valid bits in the most significant byte
	byte partialMask = ~extensionMask;

	// Is the source value negative?
	bool negative = signedInteger
		&& (partialBitCount == 0
				? srcBytes[usedSize - 1] & 0x80
				: srcBytes[usedSize - 1] & (1 << (partialBitCount - 1)));

	byte padding = negative ? byte(-1) : byte(0);

	if (destSize >= usedSize) {

		size_t i;
		for (i = 0; i < usedSize - 1; ++i)
			destBytes[i] = srcBytes[i];

		destBytes[i] = (srcBytes[i] & partialMask) | (padding & extensionMask);
		++i;

		for (; i < destSize; ++i)
			destBytes[i] = padding;
	}
	else {

		size_t i;
		for (i = 0; i < destSize; ++i)
			destBytes[i] = srcBytes[i];

		for (; i < usedSize - 1; ++i)
			if (srcBytes[i] != padding)
				throw Exception(ExceptionCode::Overflow);

		if ((srcBytes[i] & partialMask) != (padding & partialMask))
			throw Exception(ExceptionCode::Overflow);
	}
}

bool IntegerCheckIntegrity(void const *buffer, size_t bufferSize, size_t wordWidth, bool signedInteger) noexcept
{
	using byte = unsigned char;

	auto *bufferBytes = static_cast<byte const *>(buffer);

	// Quick return: a zero word width corresponds to the value zero and is valid.
	if (wordWidth == 0)
		return true;

	// Confirm that the size of the source buffer fits to the specified word width
	size_t usedSize = (wordWidth + 7) / 8;

	if (bufferSize < usedSize)
		return false;

	// The number of valid bits in the most significant byte, or 0 if all bits are valid.
	size_t partialBitCount = wordWidth % 8;

	// Mask for the extension bits in the most significant byte
	byte extensionMask = partialBitCount == 0 ? 0 : byte(-1) << partialBitCount;

	// Is the source value negative?
	bool negative = signedInteger
		&& (partialBitCount == 0
				? bufferBytes[usedSize - 1] & 0x80
				: bufferBytes[usedSize - 1] & (1 << (partialBitCount - 1)));

	byte padding = negative ? byte(-1) : byte(0);

	size_t i = usedSize - 1;

	if ((bufferBytes[i] & extensionMask) != (padding & extensionMask))
		return false;

	++i;

	for (; i < bufferSize; ++i)
		if (bufferBytes[i] != padding)
			return false;

	return true;
}

bool IntegerFixIntegrity(void *buffer, size_t bufferSize, size_t wordWidth, bool signedInteger)
{
	if (IntegerCheckIntegrity(buffer, bufferSize, wordWidth, signedInteger))
		return true;
	else {

		// `IntegerCopy` will fix any errors in the representation.
		IntegerCopy(buffer, bufferSize, buffer, bufferSize, wordWidth, signedInteger);
		return false;
	}
}

} // namespace oddf::utility
