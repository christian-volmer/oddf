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

	Signal() allows a normal C++ variable to provide a value to a node in
	the design.

*/

#pragma once

namespace dfx {
namespace blocks {

#define DECLARE_SIGNAL_FUNCTION(_type_) \
	node<_type_> Signal(_type_ const *variable); 

DECLARE_SIGNAL_FUNCTION(bool)
DECLARE_SIGNAL_FUNCTION(double)
DECLARE_SIGNAL_FUNCTION(std::int32_t)
DECLARE_SIGNAL_FUNCTION(std::int64_t)

#undef DECLARE_SIGNAL_FUNCTION

}
}
