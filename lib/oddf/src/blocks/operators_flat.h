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

	Flat operators are operators that can in principle be extended to any
	number of operands. This file provides Plus(), Sum(), Times(), Or(),
	ReductionOr(), And(), ReductionAnd(), Xor(), ReductionXor(), the
	operator forms +, *, +, ||, &&, !=, and the derived operator -.

*/

#pragma once




namespace dfx {
namespace blocks {

//
// or, and, xor
//

#define DECLARE_BOOLEAN_FUNCTION(_name_) \
	node<bool> _name_(node<bool> const &op1, node<bool> const &op2); \
	bus<bool> _name_(bus_access<bool> const &op1, bus_access<bool> const &op2); \
	node<bool> Reduction##_name_(bus_access<bool> const &operand); \

DECLARE_BOOLEAN_FUNCTION(Or)
DECLARE_BOOLEAN_FUNCTION(And)
DECLARE_BOOLEAN_FUNCTION(Xor)

#undef DECLARE_BOOLEAN_FUNCTION


//
// plus
//

#define DECLARE_PLUS_FUNCTION(_type_) \
	node<_type_> Plus(node<_type_> const &op1, node<_type_> const &op2); \
	bus<_type_> Plus(bus_access<_type_> const &op1, bus_access<_type_> const &op2); \
	node<_type_> Sum(bus_access<_type_> const &operand); \

DECLARE_PLUS_FUNCTION(double)
DECLARE_PLUS_FUNCTION(std::int32_t)
DECLARE_PLUS_FUNCTION(std::int64_t)
DECLARE_PLUS_FUNCTION(dynfix)

#undef DECLARE_PLUS_FUNCTION


//
// times
//

#define DECLARE_TIMES_FUNCTION(_type_) \
	node<_type_> Times(node<_type_> const &op1, node<_type_> const &op2); \
	bus<_type_> Times(bus_access<_type_> const &op1, bus_access<_type_> const &op2);

DECLARE_TIMES_FUNCTION(double)
DECLARE_TIMES_FUNCTION(std::int32_t)
DECLARE_TIMES_FUNCTION(std::int64_t)
DECLARE_TIMES_FUNCTION(dynfix)

#undef DECLARE_TIMES_FUNCTION

}

//
// ||, &&, !=
//

#define DEFINE_BOOLEAN_OPERATOR(_op_, _name_) \
	node<bool> inline operator _op_(node<bool> const &op1, node<bool> const &op2) \
	{ \
		return blocks::_name_(op1, op2); \
	} \
 \
 	bus<bool> inline operator _op_(bus_access<bool> const &op1, bus_access<bool> const &op2) \
	{ \
		return blocks::_name_(op1, op2); \
	}

DEFINE_BOOLEAN_OPERATOR(||, Or)
DEFINE_BOOLEAN_OPERATOR(&&, And)
DEFINE_BOOLEAN_OPERATOR(!=, Xor)

#undef DEFINE_BOOLEAN_OPERATOR


//
// +, *
//

#define DEFINE_ARITHMETIC_OPERATOR(_op_, _name_, _type_) \
	/* node _op_ node */ \
	node<_type_> inline operator _op_(node<_type_> const &op1, node<_type_> const &op2) \
	{ \
		return blocks::_name_(op1, op2); \
	} \
 \
	/* node _op_ constant */ \
	node<_type_> inline operator _op_(node<_type_> const &op1, _type_ const &op2) \
	{ \
		return blocks::_name_(op1, blocks::Constant(op2)); \
	} \
 \
	/* constant _op_ node */ \
	node<_type_> inline operator _op_(_type_ const &op1, node<_type_> const &op2) \
	{ \
		return blocks::_name_(blocks::Constant(op1), op2); \
	} \
 \
	/* bus _op_ bus */ \
	bus<_type_> inline operator _op_(bus_access<_type_> const &op1, bus_access<_type_> const &op2) \
	{ \
		return blocks::_name_(op1, op2); \
	} \
 \
	/* bus _op_ node */ \
	bus<_type_> inline operator _op_(bus_access<_type_> const &op1, node<_type_> const &op2) \
	{ \
		return blocks::_name_(op1, bus<_type_>(op2, op1.width())); \
	} \
 \
	/* node _op_ bus */ \
	bus<_type_> inline operator _op_(node<_type_> const &op1, bus_access<_type_> const &op2) \
	{ \
		return blocks::_name_(bus<_type_>(op1, op2.width()), op2); \
	} \
 \
	/* bus _op_ constant */ \
	bus<_type_> inline operator _op_(bus_access<_type_> const &op1, _type_ const &op2) \
	{ \
		return blocks::_name_(op1, bus<_type_>(blocks::Constant(op2), op1.width())); \
	} \
 \
	/* constant _op_ bus */ \
	bus<_type_> inline operator _op_(_type_ const &op1, bus_access<_type_> const &op2) \
	{ \
		return blocks::_name_(bus<_type_>(blocks::Constant(op1), op2.width()), op2); \
	}

DEFINE_ARITHMETIC_OPERATOR(+, Plus, double)
DEFINE_ARITHMETIC_OPERATOR(+, Plus, std::int32_t)
DEFINE_ARITHMETIC_OPERATOR(+, Plus, std::int64_t)
DEFINE_ARITHMETIC_OPERATOR(+, Plus, dynfix)

DEFINE_ARITHMETIC_OPERATOR(*, Times, double)
DEFINE_ARITHMETIC_OPERATOR(*, Times, std::int32_t)
DEFINE_ARITHMETIC_OPERATOR(*, Times, std::int64_t)
DEFINE_ARITHMETIC_OPERATOR(*, Times, dynfix)

#undef DEFINE_ARITHMETIC_OPERATOR


//
// -
//

#define DEFINE_SUBTRACT_OPERATOR(_type_) \
	/* node - node */ \
	node<_type_> inline operator -(node<_type_> const &op1, node<_type_> const &op2) \
	{ \
		return blocks::Plus(op1, blocks::Negate(op2)); \
	} \
 \
	/* node - constant */ \
	node<_type_> inline operator -(node<_type_> const &op1, _type_ const &op2) \
	{ \
		return blocks::Plus(op1, blocks::Constant(-op2)); \
	} \
 \
	/* constant - node */ \
	node<_type_> inline operator -(_type_ const &op1, node<_type_> const &op2) \
	{ \
		return blocks::Plus(blocks::Constant(op1), blocks::Negate(op2)); \
	} \
 \
	/* bus - bus */ \
	bus<_type_> inline operator -(bus_access<_type_> const &op1, bus_access<_type_> const &op2) \
	{ \
		return blocks::Plus(op1, blocks::Negate(op2)); \
	} \
\
	/* bus - constant */ \
	bus<_type_> inline operator -(bus_access<_type_> const &op1, _type_ const &op2) \
	{ \
		return blocks::Plus(op1, bus<_type_>(blocks::Constant(-op2), op1.width())); \
	} \
\
	/* constant - bus */ \
	bus<_type_> inline operator -(_type_ const &op1, bus_access<_type_> const &op2) \
	{ \
		return blocks::Plus(bus<_type_>(blocks::Constant(op1), op2.width()), blocks::Negate(op2)); \
	}

DEFINE_SUBTRACT_OPERATOR(double)
DEFINE_SUBTRACT_OPERATOR(std::int32_t)
DEFINE_SUBTRACT_OPERATOR(std::int64_t)
DEFINE_SUBTRACT_OPERATOR(dynfix)

#undef DEFINE_SUBTRACT_OPERATOR

//
// divide by power-of-two
//

bus<dynfix> inline operator /(bus_access<dynfix> const &op1, double op2)
{
	dynfix reciprocal(1 / op2);

	if (reciprocal.GetWordWidth() == 1)
		return op1 * reciprocal;
	else
		throw design_error("The / operator requires the divisor to be a power of two");
}

node<dynfix> inline operator /(node<dynfix> const &op1, double op2)
{
	return (bus<dynfix>(op1) / op2).first();
}

bus<double> inline operator /(bus_access<double> const &op1, double op2)
{
	dynfix reciprocal(1 / op2);

	if (reciprocal.GetWordWidth() == 1)
		return op1 * (double)reciprocal;
	else
		throw design_error("The / operator requires the divisor to be a power of two");
}

node<double> inline operator /(node<double> const &op1, double op2)
{
	return (bus<double>(op1) / op2).first();
}

}
