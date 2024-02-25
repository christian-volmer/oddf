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

	Probe() allows a normal C++ variable to probe the value of the
	provided node during simulation.

*/

#pragma once

namespace dfx {
namespace blocks {

#define DECLARE_PROBE_FUNCTION(_type_) \
	 _type_ const *Probe(node<_type_> const &theNode); 

DECLARE_PROBE_FUNCTION(bool)
DECLARE_PROBE_FUNCTION(double)
DECLARE_PROBE_FUNCTION(std::int32_t)
DECLARE_PROBE_FUNCTION(std::int64_t)

#undef DECLARE_PROBE_FUNCTION

}
}
