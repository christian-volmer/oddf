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

	Module() performs a modulo operation with offset. Implementation for
	'dynfix'.

*/

#include "../global.h"

namespace dfx {
namespace blocks {

node<dynfix> Modulo(node<dynfix> const &operand, dynfix divisor, dynfix offset)
{
	return Modulo(bus<dynfix>(operand), divisor, offset).first();
}

bus<dynfix> Modulo(bus_access<dynfix> const &operand, dynfix divisor, dynfix offset)
{
	if (operand.width() == 0)
		return operand;

	// offset == -divisor/2
	if (
		divisor.GetWordWidth() == 1 &&
		divisor.IsSigned() == false &&
		divisor.data[0] == 1 &&
		offset.GetWordWidth() == 1 &&
		offset.IsSigned() == true &&
		offset.data[0] == -1 &&
		divisor.GetFraction() + 1 == offset.GetFraction()) {

		int fraction = operand[0].GetDriver()->value.GetFraction();

		for (int i = 1; i < operand.width(); ++i)
			if (operand[1].GetDriver()->value.GetFraction() != fraction)
				throw design_error("dfx::blocks::Modulo: fractional part of all elements of the bus must be the same.");

		dynfix targetType(true, fraction - divisor.GetFraction(), fraction);
		return blocks::FloorCast(targetType, operand);

	}
	// offset == 0
	else if (
		divisor.GetWordWidth() == 1 &&
		divisor.IsSigned() == false &&
		divisor.data[0] == 1 &&
		offset.GetWordWidth() < 32 &&
		offset.IsSigned() == false &&
		offset.data[0] == 0) {

		int fraction = operand[0].GetDriver()->value.GetFraction();

		for (int i = 1; i < operand.width(); ++i)
			if (operand[1].GetDriver()->value.GetFraction() != fraction)
				throw design_error("dfx::blocks::Modulo: fractional part of all elements of the bus must be the same.");

		dynfix targetType(false, fraction - divisor.GetFraction(), fraction);
		return blocks::FloorCast(targetType, operand);
	}
	else
		throw design_error("dfx::blocks::Modulo: operation is not supported with the given values for 'divisor' and 'offset'.");
}

}
}
