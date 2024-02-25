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

	Implementation of arithmetic operations on fixed-point numbers.

*/

#include "global.h"

#include "types.h"
#include "messages.h"

namespace dfx {

//
// dynfix
//

dynfix::dynfix() :
wordWidth(0),
fraction(0),
data()
{
}

dynfix::dynfix(bool isSigned, int theWordWidth, int theFraction) :
wordWidth(isSigned ? -theWordWidth : theWordWidth),
fraction(theFraction),
data()
{
	if (theWordWidth <= 0)
		throw design_error("dfx::dynfix: The parameter 'theWordWidth' must be at least 1.");

	if (theWordWidth > MAX_FIELDS * 32)
		throw design_error("dfx::dynfix: The parameter 'theWordWidth' exceeds the maximum supported width (currently " + std::to_string(MAX_FIELDS * 32) + "). Increase dfx::dynfix::MAX_FIELDS if needed.");
}

dynfix::dynfix(std::int32_t value)
{
	Construct(value);
}
dynfix::dynfix(std::int64_t value)
{
	Construct(value);
}

dynfix::dynfix(double value)
{
	int exp = 0;
	value = std::frexp(value, &exp);

	if (value == 0.0)
		Construct(0);
	else {

		std::int64_t value64 = (std::int64_t)(value * (INT64_C(1) << 60));

		Construct(value64);
		fraction += 60 - exp;
	}
}

bool dynfix::IsInitialised() const
{
	return wordWidth != 0;
}

bool dynfix::IsSigned() const
{
	return wordWidth < 0;
}

int dynfix::GetWordWidth() const
{
	return wordWidth >= 0 ? wordWidth : -wordWidth;
}

int dynfix::GetFraction() const
{
	return fraction;
}

void dynfix::Construct(std::int64_t value)
{
	wordWidth = 0;
	fraction = 0;

	if (value != 0) {
	
		while ((value % 2) == 0) {

			--fraction;
			value /= 2;
		}
	}

	data[0] = (std::int32_t)(value & 0xffffffff);
	data[1] = (std::int32_t)((value >> 32) & 0xffffffff);

	if (value >= 0) {

		for (int i = 2; i < MAX_FIELDS; ++i)
			data[i] = 0;

		do {

			value /= 2;
			++wordWidth;

		} while (value > 0);
	}
	else if (value < INT64_C(0xc000000000000000)) {

		for (int i = 2; i < MAX_FIELDS; ++i)
			data[i] = -1;

		wordWidth = -64;
	}
	else {

		for (int i = 2; i < MAX_FIELDS; ++i)
			data[i] = -1;

		value = -value - 1;

		wordWidth = -1;
		while (value > 0) {

			value /= 2;
			--wordWidth;
		}
	}
}

dynfix dynfix::GetMin() const
{
	dynfix temp = *this;

	if (IsSigned()) {

		temp.data[0] = 1;
		for (int i = 1; i < MAX_FIELDS; ++i)
			temp.data[i] = 0;

		dynfix temp2 = temp;
		temp.CopyShiftLeft(temp2, GetWordWidth() - 1);
		temp2.OverflowWrapAround();

		return temp2;
	}
	else {

		for (int i = 0; i < MAX_FIELDS; ++i)
			temp.data[i] = 0;

		return temp;
	}
}

dynfix dynfix::GetMax() const
{
	dynfix temp = *this;

	if (IsSigned()) {
		temp.data[0] = 1;
		for (int i = 1; i < MAX_FIELDS; ++i)
			temp.data[i] = 0;

		dynfix temp2 = temp;
		temp.CopyShiftLeft(temp2, GetWordWidth() - 1);
		temp2.OverflowWrapAround();
		temp2.CopyNot(temp);

		return temp;
	}
	else {

		for (int i = 0; i < MAX_FIELDS; ++i)
			temp.data[i] = -1;

		temp.OverflowWrapAround();
		return temp;
	}
}


/*

//Should be faster but it's not in release build.

void dynfix::CopyShiftLeft(dynfix &dest, int amount) const
{
	int blockShift = amount / 32;
	int fineShift = amount % 32
	std::int32_t mask = ~((-1) << fineShift);

	for (int i = 0; i < blockShift; ++i)
		dest.data[i] = 0;

	dest.data[blockShift] = data[0] << fineShift;

	for (int i = blockShift + 1; i < MAX_FIELDS; ++i)
		dest.data[i] = (data[i - blockShift] << fineShift) | ((data[i - blockShift - 1] >> (32 - fineShift)) & mask);
}

*/


void dynfix::CopyShiftLeft(dynfix &dest, int amount) const
{
	// TODO: speed this up

	// Copy
	for (int i = MAX_FIELDS - 1; i >= 0; --i)
		dest.data[i] = data[i];

	// Shift multiples of 32 bits
	while (amount >= 32) {

		for (int i = MAX_FIELDS - 1; i > 0; --i)
			dest.data[i] = dest.data[i - 1];
		dest.data[0] = 0;
		amount -= 32;
	}

	// Do the remaining shift
	if (amount > 0) {

		std::int32_t mask = ~((-1) << amount);

		for (int i = MAX_FIELDS - 1; i > 0; --i)
			dest.data[i] = (dest.data[i] << amount) | ((dest.data[i - 1] >> (32 - amount)) & mask);

		dest.data[0] <<= amount;
	}
}

void dynfix::Copy(dynfix &dest) const
{
	for (int i = 0; i < MAX_FIELDS; ++i)
		dest.data[i] = data[i];
}

void dynfix::CopyNegate(dynfix &dest) const
{
	bool carry = true;
	for (int i = 0; i < MAX_FIELDS; ++i) {

		if (carry) {

			if (data[i] == 0) {

				dest.data[i] = 0;
				carry = true;
			}
			else {

				dest.data[i] = ~data[i] + 1;
				carry = false;
			}
		}
		else {

			dest.data[i] = ~data[i];
			carry = false;
		}
	}
}

void dynfix::CopyNot(dynfix &dest) const
{
	for (int i = 0; i < MAX_FIELDS; ++i)
		dest.data[i] = ~data[i];

}

void dynfix::AccumulateShiftLeft(dynfix &accumulator, int amount) const
{
	assert(amount >= 0 && amount < MAX_FIELDS * 32);

	int blockShift = amount / 32;
	int fineShift = amount % 32;

	std::uint32_t carry = 0;

	if (fineShift != 0) {

		std::uint32_t mask = ~(UINT32_MAX << fineShift);

		for (int i = blockShift; i < MAX_FIELDS; ++i) {

			std::uint32_t &dest = reinterpret_cast<std::uint32_t &>(accumulator.data[i]);
			std::uint32_t rhsShifted = data[i - blockShift];
			std::uint32_t rhsShiftedOut = (rhsShifted >> (32 - amount)) & mask;
			rhsShifted <<= fineShift;

			dest += carry;
			carry = dest < carry;

			dest += rhsShifted;
			carry += (dest < rhsShifted) + rhsShiftedOut;
		}
	}
	else {

		for (int i = blockShift; i < MAX_FIELDS; ++i) {

			std::uint32_t &dest = reinterpret_cast<std::uint32_t &>(accumulator.data[i]);
			std::uint32_t rhs = data[i - blockShift];

			dest += carry;
			carry = dest < carry;

			dest += rhs;
			carry += dest < rhs;
		}
	}
}

void dynfix::CopyShiftRight(dynfix &dest, int amount) const
{
	assert(amount >= 0);

	int blockShift = amount / 32;
	int fineShift = amount % 32;

	std::int32_t extension = IsSigned() && (data[MAX_FIELDS-1] < 0) ? -1 : 0;

	if (amount < MAX_FIELDS * 32) {

		for (int i = MAX_FIELDS - 1; i > MAX_FIELDS - 1 - blockShift; --i)
			dest.data[i] = extension;

		if (fineShift != 0) {

			std::int32_t mask = ~((-1) << (32 - fineShift));
			std::int32_t carry = extension << (32 - fineShift);
			for (int i = MAX_FIELDS - 1 - blockShift; i >= 0; --i) {

				dest.data[i] = ((data[i + blockShift] >> fineShift) & mask) | carry;
				carry = data[i + blockShift] << (32 - fineShift);
			}
		}
		else {

			for (int i = MAX_FIELDS - 1 - blockShift; i >= 0; --i)
				dest.data[i] = data[i + blockShift];
		}
	}
	else {

		for (int i = 0; i < MAX_FIELDS; ++i)
			dest.data[i] = extension;
	}
}

void dynfix::CopyMultiplyUnsigned(dynfix &dest, std::uint32_t m) const
{
	std::uint32_t carry = 0;
	for (int i = 0; i < dynfix::MAX_FIELDS; ++i) {

		std::uint32_t x = (std::uint32_t)data[i];
		std::uint64_t y = m * (std::uint64_t)x + (std::uint64_t)carry;

		x = y & UINT32_MAX;

		carry = y >> 32;
		dest.data[i] = x;
	}
}

void dynfix::AccumulateMultiplyUnsigned(dynfix &dest, std::uint32_t m, int block) const
{
	std::uint32_t carry = 0;
	for (int i = 0; i < dynfix::MAX_FIELDS - block; ++i) {

		std::uint32_t x = (std::uint32_t)data[i];
		std::uint64_t y = m * (std::uint64_t)x + (std::uint64_t)carry;

		x = y & UINT32_MAX;

		std::uint32_t r = dest.data[i + block];
		r += x;

		carry = (r < x) + (y >> 32);
		dest.data[i + block] = r;
	}
}

void dynfix::AccumulateMultiplySigned(dynfix &dest, std::int32_t m, int block) const
{
	std::uint32_t carry = 0;
	for (int i = 0; i < dynfix::MAX_FIELDS - block; ++i) {

		std::uint32_t x = (std::uint32_t)data[i];
		std::int64_t y = m * (std::int64_t)x + (std::int64_t)carry;

		x = y & UINT32_MAX;

		std::uint32_t r = dest.data[i + block];
		r += x;

		carry = (r < x) + (y >> 32);
		dest.data[i + block] = r;
	}
}

int dynfix::CompareUnsigned(dynfix const &rhs) const
{
	for (int i = MAX_FIELDS - 1; i >= 0; --i) {

		std::uint32_t left = (std::uint32_t)data[i];
		std::uint32_t right = (std::uint32_t)rhs.data[i];

		if (left < right)
			return -1;
		else if (left > right)
			return 1;
	}

	return 0;
}

bool dynfix::CompareEqual(dynfix const &rhs) const
{
	for (int i = MAX_FIELDS - 1; i >= 0; --i) {

		if (data[i] != rhs.data[i])
			return false;
	}

	return true;
}

int dynfix::CompareSigned(dynfix const &rhs) const
{
	{
		std::int32_t left = data[MAX_FIELDS - 1];
		std::int32_t right = rhs.data[MAX_FIELDS - 1];

		if (left < right)
			return -1;
		else if (left > right)
			return 1;
	}

	for (int i = MAX_FIELDS - 2; i >= 0; --i) {

		std::uint32_t left = (std::uint32_t)data[i];
		std::uint32_t right = (std::uint32_t)rhs.data[i];

		if (left < right)
			return -1;
		else if (left > right)
			return 1;
	}

	return 0;
}

void dynfix::OverflowWrapAround()
{
	// TODO: does this really handle and detect all possible cases?

	int highestIndex = GetWordWidth() - 1;
	int currentBlock = MAX_FIELDS - 1;
	int currentIndex = currentBlock * 32;

	if (IsSigned() && ((data[highestIndex / 32] & (1 << (highestIndex % 32))) != 0)) {

		// set all higher bits
		while (currentIndex > highestIndex) {

			data[currentBlock] = -1;
			currentIndex -= 32;
			--currentBlock;
		}

		data[currentBlock] |= (-2) << (highestIndex - currentIndex);
	}
	else {

		// clear all higher bits
		while (currentIndex > highestIndex) {

			data[currentBlock] = 0;
			currentIndex -= 32;
			--currentBlock;
		}

		data[currentBlock] &= ~((-2) << (highestIndex - currentIndex));
	}
}

dynfix dynfix::operator-() const
{
	dynfix result(true, GetWordWidth() + 1, GetFraction());
	CopyNegate(result);
	return result;
}

dynfix::operator double() const
{
	double value = 0.0;
	if (IsSigned() && (data[MAX_FIELDS - 1] < 0)) {

		value = 1.0;
		for (int i = 0; i < MAX_FIELDS; ++i)
			value += ~(std::uint32_t)data[i] * std::pow(2.0, i * 32);

		value = -value;
	}
	else {

		for (int i = 0; i < MAX_FIELDS; ++i)
			value += (std::uint32_t)data[i] * std::pow(2.0, i * 32);
	}

	return value * std::pow(2.0, -GetFraction());
}

dynfix::operator std::int64_t() const
{
	if (GetFraction() == 0 && ((IsSigned() && GetWordWidth() <= 64) || (!IsSigned() && GetWordWidth() <= 63)))
		return (static_cast<std::int64_t>(data[1]) * 0x100000000) + static_cast<std::uint32_t>(data[0]);
	else
		throw std::bad_cast();
}



//
// TypeDesc
//

namespace types {

TypeDescription::TypeDescription() :
typeInfo(0)
{
}

TypeDescription::TypeDescription(Class typeClass)
{
	if (typeClass == Boolean || typeClass == Double || typeClass == Int32 || typeClass == Int64)
		typeInfo = (unsigned)typeClass << 28;
	else
		throw design_error("TypeDescription::TypeDescription(typeClass): this constructor supports Boolean, Double, Int32, and Int64 type classes only.");
}

TypeDescription::TypeDescription(Class typeClass, bool isSigned, int wordLength, int fraction)
{
	// TODO: check arguments are inside the supported range.
	if (typeClass == FixedPoint) {

		typeInfo = ((isSigned ? 6 : 5) << 28)
			| ((wordLength & 0xfff) << 16)
			| ((fraction & 0xffff) << 0);
	}
	else
		throw design_error("TypeDescription::TypeDescription(typeClass, isSigned, wordLength, fraction): this constructor is for type class FixedPoint only.");
}


TypeDescription::Class TypeDescription::GetClass() const
{
	// class is encoded in the highest 4 bits of typeInfo.
	int typeClass = (typeInfo & 0xf0000000) >> 28;

	if (typeClass <= 4)
		return Class(typeClass);
	else if (typeClass == 5) // 5 -> unsigned fixed point
		return Class(FixedPoint);
	else if (typeClass == 6) // 6 -> signed fixed point
		return Class(FixedPoint);
	else
		return Unknown;
}

bool TypeDescription::IsClass(Class typeClass) const
{
	return GetClass() == typeClass;
}

bool TypeDescription::IsKnown() const
{
	return GetClass() != Class::Unknown;
}

bool TypeDescription::IsSigned() const
{
	// class is encoded in the highest 4 bits of typeInfo.
	int typeClass = (typeInfo & 0xf0000000) >> 28;

	if (typeClass == 5) // 5 -> unsigned fixed point
		return false;
	else if (typeClass == 6) // 6 -> signed fixed point
		return true;
	else
		throw design_error("TypeDescription::IsSigned(): can only be called on fixed-point type.");
}

int TypeDescription::GetWordWidth() const
{
	if (IsClass(FixedPoint))
		return (typeInfo & 0x0fff0000) >> 16;
	else
		throw design_error("TypeDescription::GetWordWidth(): can only be called on fixed-point type.");
}

int TypeDescription::GetFraction() const
{
	if (IsClass(FixedPoint))
		return (std::int16_t)(typeInfo & 0x0000ffff);
	else
		throw design_error("TypeDescription::GetFraction(): can only be called on fixed-point type.");
}

std::size_t TypeDescription::GetHash() const
{
	return std::hash<std::uint32_t>()(typeInfo);
}

std::string TypeDescription::ToString() const
{
	switch (GetClass()) {

		case Unknown: return "<unknown>";
		case Boolean: return "bool";
		case Double: return "double";
		case Int32: return "int32";
		case Int64: return "int64";
		case FixedPoint:
			if (IsSigned())
				return "sfix<" + std::to_string(GetWordWidth()) + ", " + std::to_string(GetFraction()) + ">";
			else
				return "ufix<" + std::to_string(GetWordWidth()) + ", " + std::to_string(GetFraction()) + ">";
	}

	assert(false);
	return "<error>";
}

bool TypeDescription::operator ==(TypeDescription const &rhs) const
{
	return typeInfo == rhs.typeInfo;
}

bool TypeDescription::operator !=(TypeDescription const &rhs) const
{
	return !(*this == rhs);
}

}
}
