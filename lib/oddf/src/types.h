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

	Constructors for sfix and ufix. Helper functions and trait types for
	the data types supported by the framework.

*/

#pragma once

#include "type_support.h"

#include <cstdint>
#include <algorithm>

namespace dfx {

//
// fixed-point support
//

template<typename InputIt> inline dynfix dynfix::CommonRepresentation(InputIt first, InputIt last)
{
	// If any of the operands is signed, the result will also be signed.
	bool isSigned = std::any_of(first, last, [](dynfix const &x) { return x.IsSigned(); });

	// Align the position of the comma: the fractional part of the result will be the largest fraction occuring in the operands.
	int fraction = first->GetFraction();
	for (auto it = std::next(first); it != last; ++it)
		fraction = std::max(fraction, it->GetFraction());

	// Compute the word width of the result. 
	// Unsigned operands need an extra sign bit, if the result is signed. 
	// Fractional part must be padded to the fractional length of the result.
	auto wordWidthFunction = [=](dynfix const &x) {

		return x.GetWordWidth()
			+ (isSigned && !x.IsSigned() ? 1 : 0)
			+ (fraction - x.GetFraction());
	};

	// Determine the largest adjusted word width.
	int wordWidth = wordWidthFunction(*first);
	for (auto it = std::next(first); it != last; ++it)
		wordWidth = std::max(wordWidth, wordWidthFunction(*it));

	return dynfix(isSigned, wordWidth, fraction);
}


//
// sfix
//

template<int wordWidthArg, int fractionArg> inline sfix<wordWidthArg, fractionArg>::sfix() :
	dynfix(true, wordWidthArg, fractionArg)
{
}

template<int wordWidthArg, int fractionArg> inline sfix<wordWidthArg, fractionArg>::sfix(std::int32_t value) :
	dynfix(true, wordWidthArg, fractionArg)
{
	dynfix temp(value);
	int align = GetFraction() - temp.GetFraction();
	if (align >= 0)
		temp.CopyShiftLeft(*this, align);
	else
		temp.CopyShiftRight(*this, -align);

	OverflowWrapAround();
}

template<int wordWidthArg, int fractionArg> inline sfix<wordWidthArg, fractionArg>::sfix(std::int64_t value) :
	dynfix(true, wordWidthArg, fractionArg)
{
	dynfix temp(value);
	int align = GetFraction() - temp.GetFraction();
	if (align >= 0)
		temp.CopyShiftLeft(*this, align);
	else
		temp.CopyShiftRight(*this, -align);

	OverflowWrapAround();
}

template<int wordWidthArg, int fractionArg> inline sfix<wordWidthArg, fractionArg>::sfix(double value) :
	dynfix(true, wordWidthArg, fractionArg)
{
	dynfix temp(value);
	int align = GetFraction() - temp.GetFraction();
	if (align >= 0)
		temp.CopyShiftLeft(*this, align);
	else
		temp.CopyShiftRight(*this, -align);

	OverflowWrapAround();
}


//
// ufix
//

template<int wordWidthArg, int fractionArg> inline ufix<wordWidthArg, fractionArg>::ufix() :
	dynfix(false, wordWidthArg, fractionArg)
{
}

template<int wordWidthArg, int fractionArg> inline ufix<wordWidthArg, fractionArg>::ufix(std::int32_t value) :
	dynfix(false, wordWidthArg, fractionArg)
{
	dynfix temp(value);
	int align = GetFraction() - temp.GetFraction();
	if (align >= 0)
		temp.CopyShiftLeft(*this, align);
	else
		temp.CopyShiftRight(*this, -align);

	OverflowWrapAround();
}

template<int wordWidthArg, int fractionArg> inline ufix<wordWidthArg, fractionArg>::ufix(std::int64_t value) :
	dynfix(false, wordWidthArg, fractionArg)
{
	dynfix temp(value);
	int align = GetFraction() - temp.GetFraction();
	if (align >= 0)
		temp.CopyShiftLeft(*this, align);
	else
		temp.CopyShiftRight(*this, -align);

	OverflowWrapAround();
}

template<int wordWidthArg, int fractionArg> inline ufix<wordWidthArg, fractionArg>::ufix(double value) :
	dynfix(false, wordWidthArg, fractionArg)
{
	dynfix temp(value);
	int align = GetFraction() - temp.GetFraction();
	if (align >= 0)
		temp.CopyShiftLeft(*this, align);
	else
		temp.CopyShiftRight(*this, -align);

	OverflowWrapAround();
}


namespace types {


//
// type description
//

class TypeDescription {

private:

	std::uint32_t typeInfo;

public:

	enum Class {

		Unknown = 0,
		Boolean = 1,
		Double = 2,
		Int32 = 3,
		Int64 = 4,
		FixedPoint = 5,
	};

	Class GetClass() const;
	bool IsClass(Class typeClass) const;
	bool IsKnown() const;
	bool IsSigned() const;
	int GetWordWidth() const;
	int GetFraction() const;

	std::size_t GetHash() const;
	std::string ToString() const;

	TypeDescription();
	explicit TypeDescription(Class typeClass);
	explicit TypeDescription(Class typeClass, bool isSigned, int wordLength, int fraction);

	bool operator ==(TypeDescription const &rhs) const;
	bool operator !=(TypeDescription const &rhs) const;
};


//
// type traits
//

template<typename T>
struct TypeTraits;

// bool
template<> struct TypeTraits<bool> {

	using valueType = bool;
	using internalType = bool;

	static bool IsInitialised(internalType const &)
	{
		return true;
	}

	static valueType DefaultFrom(valueType const &)
	{
		return false;
	}

	static void Copy(valueType &dest, valueType const &source)
	{
		dest = source;
	}

	static bool IsEqual(valueType &dest, valueType const &source)
	{
		return dest == source;
	}

	static TypeDescription GetDescription(valueType const &)
	{
		return TypeDescription(TypeDescription::Boolean);
	}

	static bool IsCompatible(internalType const &, internalType const &)
	{
		return true;
	}
};

// int32
template<> struct TypeTraits<std::int32_t> {

	using valueType = std::int32_t;
	using internalType = std::int32_t;

	static bool IsInitialised(internalType const &)
	{
		return true;
	}

	static valueType DefaultFrom(valueType const &)
	{
		return valueType();
	}

	static void Copy(valueType &dest, valueType const &source)
	{
		dest = source;
	}

	static bool IsEqual(valueType &dest, valueType const &source)
	{
		return dest == source;
	}

	static TypeDescription GetDescription(valueType const &)
	{
		return TypeDescription(TypeDescription::Int32);
	}

	static bool IsCompatible(internalType const &, internalType const &)
	{
		return true;
	}
};

// int64
template<> struct TypeTraits<std::int64_t> {

	using valueType = std::int64_t;
	using internalType = std::int64_t;

	static bool IsInitialised(internalType const &)
	{
		return true;
	}

	static valueType DefaultFrom(valueType const &)
	{
		return valueType();
	}

	static void Copy(valueType &dest, valueType const &source)
	{
		dest = source;
	}

	static bool IsEqual(valueType &dest, valueType const &source)
	{
		return dest == source;
	}

	static TypeDescription GetDescription(valueType const &)
	{
		return TypeDescription(TypeDescription::Int64);
	}

	static bool IsCompatible(internalType const &, internalType const &)
	{
		return true;
	}
};

// double
template<> struct TypeTraits<double> {

	using valueType = double;
	using internalType = double;

	static bool IsInitialised(internalType const &)
	{
		return true;
	}

	static valueType DefaultFrom(valueType const &)
	{
		return valueType();
	}

	static void Copy(valueType &dest, valueType const &source)
	{
		dest = source;
	}

	static bool IsEqual(valueType &dest, valueType const &source)
	{
		return dest == source;
	}

	static TypeDescription GetDescription(valueType const &)
	{
		return TypeDescription(TypeDescription::Double);
	}

	static bool IsCompatible(internalType const &, internalType const &)
	{
		return true;
	}
};

// dynfix
template<> struct TypeTraits<dynfix> {

	using valueType = dynfix;
	using internalType = dynfix;

	static bool IsInitialised(internalType const &value)
	{
		return value.IsInitialised();
	}

	static valueType DefaultFrom(valueType const &value)
	{
		return dynfix(value.IsSigned(), value.GetWordWidth(), value.GetFraction());
	}

	static void Copy(valueType &dest, valueType const &source)
	{
		source.Copy(dest);
	}

	static bool IsEqual(valueType &dest, valueType const &source)
	{
		return source.CompareEqual(dest);
	}

	static TypeDescription GetDescription(valueType const &value)
	{
		return TypeDescription(TypeDescription::FixedPoint, value.IsSigned(), value.GetWordWidth(), value.GetFraction());
	}

	static bool IsCompatible(internalType const &lhs, internalType const &rhs)
	{
		return (lhs.IsSigned() == rhs.IsSigned()) && (lhs.GetWordWidth() == rhs.GetWordWidth()) && (lhs.GetFraction() == rhs.GetFraction());
	}
};

// sfix
template<int wordWidth, int fraction> struct TypeTraits<sfix<wordWidth, fraction>> {

	using valueType = sfix<wordWidth, fraction>;
	using internalType = dynfix;

	static bool IsInitialised(internalType const &value)
	{
		return value.IsInitialised();
	}

	static valueType DefaultFrom(valueType const &)
	{
		return valueType();
	}

	static void Copy(valueType &dest, valueType const &source)
	{
		source.Copy(dest);
	}

	static bool IsEqual(valueType &dest, valueType const &source)
	{
		return source.CompareEqual(dest);
	}

	static TypeDescription GetDescription(valueType const &value)
	{
		return TypeDescription(TypeDescription::FixedPoint, value.IsSigned(), value.GetWordWidth(), value.GetFraction());
	}
};

// ufix
template<int wordWidth, int fraction> struct TypeTraits<ufix<wordWidth, fraction>> {

	using valueType = ufix<wordWidth, fraction>;
	using internalType = dynfix;

	static bool IsInitialised(internalType const &value)
	{
		return value.IsInitialised();
	}

	static valueType DefaultFrom(valueType const &)
	{
		return valueType();
	}

	static void Copy(valueType &dest, valueType const &source)
	{
		source.Copy(dest);
	}

	static bool IsEqual(valueType &dest, valueType const &source)
	{
		return source.CompareEqual(dest);
	}

	static TypeDescription GetDescription(valueType const &value)
	{
		return TypeDescription(TypeDescription::FixedPoint, value.IsSigned(), value.GetWordWidth(), value.GetFraction());
	}
};

template<typename T> using nodeType = typename dfx::types::TypeTraits<T>::internalType;

template<typename T>
bool IsInitialised(T const &value)
{
	return TypeTraits<T>::IsInitialised(value);
}

template<typename T>
T DefaultFrom(T const &value)
{
	assert(IsInitialised(value));
	return TypeTraits<T>::DefaultFrom(value);
}

template<typename T>
TypeDescription GetDescription(T const &value)
{
	return TypeTraits<T>::GetDescription(value);
}

template<typename T>
void Copy(T &dest, T const &source)
{
	TypeTraits<T>::Copy(dest, source);
}

template<typename T>
bool IsEqual(T &dest, T const &source)
{
	return TypeTraits<T>::IsEqual(dest, source);
}

template<typename T>
bool IsCompatible(T const &lhs, T const &rhs)
{
	return TypeTraits<T>::IsCompatible(lhs, rhs);
}

}
}
