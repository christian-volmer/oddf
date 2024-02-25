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

	Relational operators are ==, !=, <, >, <=, >=.

*/

#pragma once

namespace dfx {
namespace blocks {


//
// Equal, Less, LessEqual
//

#define DECLARE_RELATIONAL_FUNCTIONS(_name_, _type_) \
	node<bool> _name_(node<_type_> const &leftOperand, node<_type_> const &rightOperand); \
	bus<bool> _name_(bus_access<_type_> const &leftOperand, bus_access<_type_> const &rightOperand);

DECLARE_RELATIONAL_FUNCTIONS(Equal, bool)
DECLARE_RELATIONAL_FUNCTIONS(Equal, double)
DECLARE_RELATIONAL_FUNCTIONS(Equal, std::int32_t)
DECLARE_RELATIONAL_FUNCTIONS(Equal, std::int64_t)
DECLARE_RELATIONAL_FUNCTIONS(Equal, dynfix)

// DECLARE_RELATIONAL_FUNCTIONS(NotEqual, bool) --> see 'operators_flat'.
DECLARE_RELATIONAL_FUNCTIONS(NotEqual, double)
DECLARE_RELATIONAL_FUNCTIONS(NotEqual, std::int32_t)
DECLARE_RELATIONAL_FUNCTIONS(NotEqual, std::int64_t)
DECLARE_RELATIONAL_FUNCTIONS(NotEqual, dynfix)

DECLARE_RELATIONAL_FUNCTIONS(Less, double)
DECLARE_RELATIONAL_FUNCTIONS(Less, std::int32_t)
DECLARE_RELATIONAL_FUNCTIONS(Less, std::int64_t)
DECLARE_RELATIONAL_FUNCTIONS(Less, dynfix)

DECLARE_RELATIONAL_FUNCTIONS(LessEqual, double)
DECLARE_RELATIONAL_FUNCTIONS(LessEqual, std::int32_t)
DECLARE_RELATIONAL_FUNCTIONS(LessEqual, std::int64_t)
DECLARE_RELATIONAL_FUNCTIONS(LessEqual, dynfix)

#undef DECLARE_RELATIONAL_FUNCTIONS

}


//
// ==, !=, <, <=
//

#define DEFINE_RELATIONAL_OPERATOR(_op_, _name_, _type_) \
	/* node _op_ node */ \
	node<bool> inline operator _op_(node<_type_> const &leftOperand, node<_type_> const &rightOperand) \
	{ \
		return blocks::_name_(leftOperand, rightOperand); \
	} \
 \
	/* node _op_ constant */ \
	node<bool> inline operator _op_(node<_type_> const &leftOperand, _type_ const &rightOperand) \
	{ \
		return blocks::_name_(leftOperand, blocks::Constant(rightOperand)); \
	} \
 \
	/* constant _op_ node */ \
	node<bool> inline operator _op_(_type_ const &leftOperand, node<_type_> const &rightOperand) \
	{ \
		return blocks::_name_(blocks::Constant(leftOperand), rightOperand); \
	} \
 \
	/* bus _op_ bus */ \
	bus<bool> inline operator _op_(bus_access<_type_> const &leftOperand, bus_access<_type_> const &rightOperand) \
	{ \
		return blocks::_name_(leftOperand, rightOperand); \
	} \
\
	/* bus _op_ constant */ \
	bus<bool> inline operator _op_(bus_access<_type_> const &leftOperand, _type_ const &rightOperand) \
	{ \
		return blocks::_name_(leftOperand, bus<_type_>(blocks::Constant(rightOperand), leftOperand.width())); \
	} \
\
	/* constant _op_ bus */ \
	bus<bool> inline operator _op_(_type_ const &leftOperand, bus_access<_type_> const &rightOperand) \
	{ \
		return blocks::_name_(bus<_type_>(blocks::Constant(leftOperand), rightOperand.width()), rightOperand); \
	}

DEFINE_RELATIONAL_OPERATOR(==, Equal, bool)
DEFINE_RELATIONAL_OPERATOR(==, Equal, double)
DEFINE_RELATIONAL_OPERATOR(==, Equal, std::int32_t)
DEFINE_RELATIONAL_OPERATOR(==, Equal, std::int64_t)
DEFINE_RELATIONAL_OPERATOR(==, Equal, dynfix)

// DEFINE_RELATIONAL_OPERATOR(!=, NotEqual, bool) --> this is equivalent to XOR, see 'operators_flat'.
DEFINE_RELATIONAL_OPERATOR(!=, NotEqual, double)
DEFINE_RELATIONAL_OPERATOR(!=, NotEqual, std::int32_t)
DEFINE_RELATIONAL_OPERATOR(!=, NotEqual, std::int64_t)
DEFINE_RELATIONAL_OPERATOR(!=, NotEqual, dynfix)

DEFINE_RELATIONAL_OPERATOR(<, Less, double)
DEFINE_RELATIONAL_OPERATOR(<, Less, std::int32_t)
DEFINE_RELATIONAL_OPERATOR(<, Less, std::int64_t)
DEFINE_RELATIONAL_OPERATOR(<, Less, dynfix)

DEFINE_RELATIONAL_OPERATOR(<=, LessEqual, double)
DEFINE_RELATIONAL_OPERATOR(<=, LessEqual, std::int32_t)
DEFINE_RELATIONAL_OPERATOR(<=, LessEqual, std::int64_t)
DEFINE_RELATIONAL_OPERATOR(<=, LessEqual, dynfix)

#undef DEFINE_RELATIONAL_OPERATOR


//
// >, >=
//

// The following macros define > and >= in terms of Less and LessEqual by swapping
// the operands, i.e., 'left > right' is equivalent to 'right < left'.

#define DEFINE_RELATIONAL_OPERATOR_SWAPPED(_op_, _name_, _type_) \
	/* node _op_ node */ \
	node<bool> inline operator _op_(node<_type_> const &leftOperand, node<_type_> const &rightOperand) \
	{ \
		return blocks::_name_(rightOperand, leftOperand); \
	} \
 \
	/* node _op_ constant */ \
	node<bool> inline operator _op_(node<_type_> const &leftOperand, _type_ const &rightOperand) \
	{ \
		return blocks::_name_(blocks::Constant(rightOperand), leftOperand); \
	} \
 \
	/* constant _op_ node */ \
	node<bool> inline operator _op_(_type_ const &leftOperand, node<_type_> const &rightOperand) \
	{ \
		return blocks::_name_(rightOperand, blocks::Constant(leftOperand)); \
	} \
 \
	/* bus _op_ bus */ \
	bus<bool> inline operator _op_(bus_access<_type_> const &leftOperand, bus_access<_type_> const &rightOperand) \
	{ \
		return blocks::_name_(rightOperand, leftOperand); \
	} \
\
	/* bus _op_ constant */ \
	bus<bool> inline operator _op_(bus_access<_type_> const &leftOperand, _type_ const &rightOperand) \
	{ \
		return blocks::_name_(bus<_type_>(blocks::Constant(rightOperand), leftOperand.width()), leftOperand); \
	} \
\
	/* constant _op_ bus */ \
	bus<bool> inline operator _op_(_type_ const &leftOperand, bus_access<_type_> const &rightOperand) \
	{ \
		return blocks::_name_(rightOperand, bus<_type_>(blocks::Constant(leftOperand), rightOperand.width())); \
	}

DEFINE_RELATIONAL_OPERATOR_SWAPPED(>, Less, double)
DEFINE_RELATIONAL_OPERATOR_SWAPPED(>, Less, std::int32_t)
DEFINE_RELATIONAL_OPERATOR_SWAPPED(>, Less, std::int64_t)
DEFINE_RELATIONAL_OPERATOR_SWAPPED(>, Less, dynfix)

DEFINE_RELATIONAL_OPERATOR_SWAPPED(>=, LessEqual, double)
DEFINE_RELATIONAL_OPERATOR_SWAPPED(>=, LessEqual, std::int32_t)
DEFINE_RELATIONAL_OPERATOR_SWAPPED(>=, LessEqual, std::int64_t)
DEFINE_RELATIONAL_OPERATOR_SWAPPED(>=, LessEqual, dynfix)

#undef DEFINE_RELATIONAL_OPERATOR_SWAPPED

}
