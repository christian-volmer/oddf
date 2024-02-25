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

#pragma once

namespace dfx {
namespace blocks {

node<std::int32_t> PowerOfTwo(node<std::int32_t> const &exponent);
node<std::int64_t> PowerOfTwo(node<std::int64_t> const &exponent);
node<dynfix> PowerOfTwo(node<dynfix> const &exponent);

bus<dynfix> TimesPowerOfTwo(bus_access<dynfix> const &value, node<dynfix> const &exponent, int exponentMin, int exponentMax);
bus<dynfix> TimesPowerOfTwo(bus_access<dynfix> const &value, node<dynfix> const &exponent);

node<dynfix> TimesPowerOfTwo(node<dynfix> const &value, node<dynfix> const &exponent, int exponentMin, int exponentMax);
node<dynfix> TimesPowerOfTwo(node<dynfix> const &value, node<dynfix> const &exponent);

}
}
