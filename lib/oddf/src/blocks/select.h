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

	Select() returns a sequence of elements of a bus based starting at
	the index given by its input node.

*/

#pragma once

namespace dfx {
namespace blocks {

#define DECLARE_SELECT_FUNCTION(_type_) \
	bus<_type_> Select(bus_access<_type_> const &input, node<dynfix> const &index, int length); \
	inline node<_type_> Select(bus_access<_type_> const &input, node<dynfix> const &index) \
	{ \
		return Select(input, index, 1).first(); \
	} \
	inline bus<_type_> Select(bus_access<_type_> const &input, node<int> const &index, int length) \
	{ \
		return Select(input, blocks::FloorCast<ufix<31>>(index), length); \
	} \
	inline node<_type_> Select(bus_access<_type_> const &input, node<int> const &index) \
	{ \
		return Select(input, index, 1).first(); \
	} \

DECLARE_SELECT_FUNCTION(bool)
DECLARE_SELECT_FUNCTION(double)
DECLARE_SELECT_FUNCTION(std::int32_t)
DECLARE_SELECT_FUNCTION(std::int64_t)
DECLARE_SELECT_FUNCTION(dynfix)

#undef DECLARE_SELECT_FUNCTION

}
}
