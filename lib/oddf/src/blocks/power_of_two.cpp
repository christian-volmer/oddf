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

	PowerOfTwo() and TimesPowerOfTwo() return and multiply by the number
	2 raised to the power of the input node. The exponent may be
	negative.

*/

#include "../global.h"

namespace dfx {
namespace blocks {

node<std::int32_t> PowerOfTwo(node<std::int32_t> const &exponent)
{
	return Function(exponent, [](std::int32_t exp) 
	{ 
		if (exp < 0 || exp > 30)
			throw design_error("The integer exponent is outside the permitted range [0 30] during evaluation of a PowerOfTwo operation.");
		return int32_t(1) << exp; 
	});
}

bus<std::int32_t> PowerOfTwo(bus_access<std::int32_t> const &exponent)
{
	return Function(exponent, [](std::int32_t exp) 
	{ 
		if (exp < 0 || exp > 30)
			throw design_error("The integer exponent is outside the permitted range [0 30] during evaluation of a PowerOfTwo operation.");
		return int32_t(1) << exp;
	});
}


node<std::int64_t> PowerOfTwo(node<std::int64_t> const &exponent)
{
	return Function(exponent, [](std::int64_t exp) { 
		if (exp < 0 || exp > 62)
			throw design_error("The integer exponent is outside the permitted range [0 62] during evaluation of a PowerOfTwo operation.");
		return int64_t(1) << exp;
	});
}

bus<std::int64_t> PowerOfTwo(bus_access<std::int64_t> const &exponent)
{
	return Function(exponent, [](std::int64_t exp) { 
		if (exp < 0 || exp > 62)
			throw design_error("The integer exponent is outside the permitted range [0 62] during evaluation of a PowerOfTwo operation.");
		return int64_t(1) << exp;
	});
}

node<dynfix> PowerOfTwo(node<dynfix> const &exponent)
{
	auto expTypeDesc = types::GetDescription(exponent.GetDriver()->value);

	if (expTypeDesc.GetFraction() > 0)
		throw design_error("dfx::blocks::PowerOfTwo: 'exponent' must be an integer, i.e., its fractional part must be negative or zero. Type of 'exponent' is '" + expTypeDesc.ToString() + "'.");

	if (expTypeDesc.GetWordWidth() - expTypeDesc.GetFraction() > 16)
		throw design_error("dfx::blocks::PowerOfTwo: word width of 'exponent' is too large. Result would have a word width of more than 65536 bits. Type of 'exponent' is '" + expTypeDesc.ToString() + "'.");

	int minShift;
	int maxShift;

	if (expTypeDesc.IsSigned() == false) {

		minShift = 0;
		maxShift = ((1 << expTypeDesc.GetWordWidth()) - 1) << (-expTypeDesc.GetFraction());
	}
	else {

		minShift = -(((1 << (expTypeDesc.GetWordWidth() - 1))) << (-expTypeDesc.GetFraction()));
		maxShift = ((1 << (expTypeDesc.GetWordWidth()-1)) - 1) << (-expTypeDesc.GetFraction());
	}

	dynfix targetType(false, maxShift - minShift + 1, -minShift);
	bus<bool> padding = bus<bool>(Constant(false), targetType.GetWordWidth() - 1);

	dynfix shiftType(false, expTypeDesc.GetWordWidth(), expTypeDesc.GetFraction());
	node<dynfix> shift = FloorCast(shiftType, maxShift - exponent);
	
	return BitCompose(targetType, Select(dfx::join(padding, Constant(true), padding), shift, targetType.GetWordWidth()));
}

node<dynfix> TimesPowerOfTwo(node<dynfix> const &value, node<dynfix> const &exponent)
{
	return TimesPowerOfTwo(value, exponent, -32768, 65535);
}

bus<dynfix> TimesPowerOfTwo(bus_access<dynfix> const &values, node<dynfix> const &exponent)
{
	return TimesPowerOfTwo(values, exponent, -32768, 65535);
}

node<dynfix> TimesPowerOfTwo(node<dynfix> const &value, node<dynfix> const &exponent, int exponentMin, int exponentMax)
{
	auto valueTypeDesc = types::GetDescription(value.GetDriver()->value);
	auto expTypeDesc = types::GetDescription(exponent.GetDriver()->value);

	if (expTypeDesc.GetFraction() > 0)
		throw design_error("dfx::blocks::TimesPowerOfTwo: 'exponent' must be an integer, i.e., its fractional part must be negative or zero. Type of 'exponent' is '" + expTypeDesc.ToString() + "'.");

	int minShift;
	int maxShift;

	if (expTypeDesc.GetWordWidth() - expTypeDesc.GetFraction() < 16) {

		if (expTypeDesc.IsSigned() == false) {

			minShift = 0;
			maxShift = ((1 << expTypeDesc.GetWordWidth()) - 1) << (-expTypeDesc.GetFraction());
		}
		else {

			minShift = -(((1 << (expTypeDesc.GetWordWidth() - 1))) << (-expTypeDesc.GetFraction()));
			maxShift = ((1 << (expTypeDesc.GetWordWidth() - 1)) - 1) << (-expTypeDesc.GetFraction());
		}
	}
	else {

		if (expTypeDesc.IsSigned() == false) {

			minShift = 0;
			maxShift = 65535;
		}
		else {

			minShift = -32768;
			maxShift = 32767;
		}
	}

	if (maxShift < exponentMax)
		exponentMax = maxShift;

	if (minShift > exponentMin)
		exponentMin = minShift;

	int paddingAmount = exponentMax - exponentMin;
	bus<bool> valueBits = BitExtract(value);
	bus<bool> lowerPadding = bus<bool>(Constant(false), paddingAmount);
	bus<bool> upperPadding = valueTypeDesc.IsSigned() ? bus<bool>(valueBits.last(), paddingAmount) : lowerPadding; // potential sign extension

	bus<bool> bits = dfx::join(lowerPadding, valueBits, upperPadding);

	dynfix targetType(valueTypeDesc.IsSigned(), valueTypeDesc.GetWordWidth() + paddingAmount, valueTypeDesc.GetFraction() - exponentMin);

	return BitCompose(targetType, Select(bits, exponentMax - exponent, targetType.GetWordWidth()));
}

bus<dynfix> TimesPowerOfTwo(bus_access<dynfix> const &values, node<dynfix> const &exponent, int exponentMin, int exponentMax)
{
	bus<dynfix> result(values.width());

	for (int i = 0; i < values.width(); ++i)
		result[i] = TimesPowerOfTwo(values[i], exponent, exponentMin, exponentMax);

	return result;
}

}
}
