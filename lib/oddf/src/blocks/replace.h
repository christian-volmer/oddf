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

	Replace() replaces a sequence of elements in a bus based on the given
	index and data nodes.

*/

#pragma once

namespace dfx {
namespace blocks {

#define DECLARE_REPLACE_FUNCTION(_type_) \
	bus<_type_> Replace(bus_access<_type_> const &originalItems, node<int> const &index, bus_access<_type_> const &newItems); \
	inline bus<_type_> Replace(bus_access<_type_> const &originalItems, node<int> const &index, node<_type_> const &newItem) \
	{ \
		return Replace(originalItems, index, bus<_type_>(newItem)); \
	}	

DECLARE_REPLACE_FUNCTION(bool)
DECLARE_REPLACE_FUNCTION(double)
DECLARE_REPLACE_FUNCTION(std::int32_t)
DECLARE_REPLACE_FUNCTION(std::int64_t)

#undef DECLARE_REPLACE_FUNCTION

}
}
