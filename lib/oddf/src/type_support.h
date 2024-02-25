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

	Fixed point type classes and helper classes.

*/

#pragma once

#include <cstdint>
#include <algorithm>

namespace dfx {

//
// fixed-point support
//

struct dynfix {

protected:

	int wordWidth;
	int fraction;

	void Construct(std::int64_t value);

public:

	static int const MAX_FIELDS = 4; // do not commit !!! was: 4;

	std::int32_t data[MAX_FIELDS];

public:

	dynfix();
	dynfix(bool isSigned, int theWordWidth, int theFraction);
	dynfix(std::int32_t value);
	dynfix(std::int64_t value);
	dynfix(double value);

	bool IsInitialised() const;

	bool IsSigned() const;
	int GetWordWidth() const;
	int GetFraction() const;

	dynfix GetMin() const;
	dynfix GetMax() const;

	void Copy(dynfix &dest) const;
	void CopyNegate(dynfix &dest) const;
	void CopyNot(dynfix &dest) const;
	void CopyShiftRight(dynfix &dest, int amount) const;
	void CopyShiftLeft(dynfix &dest, int amount) const;
	void CopyMultiplyUnsigned(dynfix &dest, std::uint32_t m) const;

	void AccumulateShiftLeft(dynfix &dest, int amount) const;
	void AccumulateMultiplyUnsigned(dynfix &dest, std::uint32_t m, int block) const;
	void AccumulateMultiplySigned(dynfix &dest, std::int32_t m, int block) const;

	bool CompareEqual(dynfix const &rhs) const;
	int CompareUnsigned(dynfix const &rhs) const;
	int CompareSigned(dynfix const &rhs) const;

	void OverflowWrapAround();
	
	dynfix operator-() const;
	operator double() const;
	operator std::int64_t() const;

	template<typename InputIt> static dynfix CommonRepresentation(InputIt first, InputIt last);
};

template<int wordWidthArg, int fractionArg = 0>
struct sfix : public dynfix {

	static_assert(wordWidthArg > 0, "The template argument 'wordWidthArg' of the 'sfix' data type must be at least 1.");

	sfix();

	sfix(std::int32_t value);
	sfix(std::int64_t value);
	sfix(double value);
};

template<int wordWidthArg, int fractionArg = 0>
struct ufix : public dynfix {

	static_assert(wordWidthArg > 0, "The template argument 'wordWidthArg' of the 'ufix' data type must be at least 1.");

	ufix();

	ufix(std::int32_t value);
	ufix(std::int64_t value);
	ufix(double value);
};

namespace types {

//
// Compile-time adjustments to types
//

// adjust word-width

template<typename baseT, int> struct AdjustWordWidth_t {

	using type = baseT;
};

template<int wordWidth, int fraction, int adjustment> struct AdjustWordWidth_t<ufix<wordWidth, fraction>, adjustment> {

	static_assert(wordWidth + adjustment, "AdjustWordWidth: resulting word width cannot be less than 1.");
	using type = ufix<wordWidth + adjustment, fraction>;
};

template<int wordWidth, int fraction, int adjustment> struct AdjustWordWidth_t<sfix<wordWidth, fraction>, adjustment> {

	static_assert(wordWidth + adjustment, "AdjustWordWidth: resulting word width cannot be less than 1.");
	using type = sfix<wordWidth + adjustment, fraction>;
};

template<typename baseT, int adjustment> using AdjustWordWidth = typename AdjustWordWidth_t<baseT, adjustment>::type;


// adjust fraction

template<typename baseT, int> struct AdjustFraction_t {

	using type = baseT;
};

template<int wordWidth, int fraction, int adjustment> struct AdjustFraction_t<ufix<wordWidth, fraction>, adjustment> {

	static_assert(wordWidth + adjustment, "AdjustFraction: resulting word width cannot be less than 1.");
	using type = ufix<wordWidth + adjustment, fraction + adjustment>;
};

template<int wordWidth, int fraction, int adjustment> struct AdjustFraction_t<sfix<wordWidth, fraction>, adjustment> {

	static_assert(wordWidth + adjustment, "AdjustFraction: resulting word width cannot be less than 1.");
	using type = sfix<wordWidth + adjustment, fraction + adjustment>;
};

template<typename baseT, int adjustment> using AdjustFraction = typename AdjustFraction_t<baseT, adjustment>::type;


// make singned

template<typename baseT> struct MakeSigned_t {

	using type = baseT;
};

template<int wordWidth, int fraction> struct MakeSigned_t<ufix<wordWidth, fraction>> {

	using type = sfix<wordWidth + 1, fraction>;
};

template<int wordWidth, int fraction> struct MakeSigned_t<sfix<wordWidth, fraction>> {

	using type = sfix<wordWidth, fraction>;
};

template<typename baseT> using MakeSigned = typename MakeSigned_t<baseT>::type;


// required bits

template<std::size_t number> struct RequiredBits {

	static int const value = 1 + RequiredBits<number / 2>::value;
};

template<> struct RequiredBits<0> {

	static int const value = 0;
};

}
}
