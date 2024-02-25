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

	Unary operators take exactly one operand. Provides Negate() and Not()
	and the operator forms - and !.

*/

#pragma once

namespace dfx {
namespace blocks {


//
// negate
//

#define DECLARE_NEGATE_FUNCTION(_type_) \
	node<_type_> Negate(node<_type_> const &operand); \
	bus<_type_> Negate(bus_access<_type_> const &operand);

DECLARE_NEGATE_FUNCTION(double)
DECLARE_NEGATE_FUNCTION(std::int32_t)
DECLARE_NEGATE_FUNCTION(std::int64_t)
DECLARE_NEGATE_FUNCTION(dynfix)

#undef DECLARE_NEGATE_FUNCTION


//
// not
//

node<bool> Not(node<bool> const &operand);
bus<bool> Not(bus_access<bool> const &operand);

}


//
// unary -
//

#define DEFINE_NEGATION_OPERATOR(_type_) \
	node<_type_> inline operator -(node<_type_> const &operand) \
	{ \
		return blocks::Negate(operand); \
	} \
 \
	bus<_type_> inline operator -(bus_access<_type_> const &operand) \
	{ \
		return blocks::Negate(operand); \
	}

DEFINE_NEGATION_OPERATOR(double)
DEFINE_NEGATION_OPERATOR(std::int32_t)
DEFINE_NEGATION_OPERATOR(std::int64_t)
DEFINE_NEGATION_OPERATOR(dynfix)

#undef DEFINE_NEGATION_OPERATOR


//
// !
//

node<bool> inline operator !(node<bool> const &operand)
{
	return blocks::Not(operand);
}

bus<bool> inline operator !(bus_access<bool> const &operand)
{
	return blocks::Not(operand);
}

}
