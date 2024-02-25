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

	Zero() drives the value zero (or false) into a node. Terminate()
	consumes a node as input and does nothing. Disconnect() disconnects a
	node to prevent accidental use.

*/

#pragma once

namespace dfx {
namespace blocks {

#define DECLARE_TERMINATE_FUNCTION(_type_) \
	void Terminate(node<_type_> const &input); \
	void Terminate(bus_access<_type_> const &inputBus);

DECLARE_TERMINATE_FUNCTION(bool)
DECLARE_TERMINATE_FUNCTION(double)
DECLARE_TERMINATE_FUNCTION(std::int32_t)
DECLARE_TERMINATE_FUNCTION(std::int64_t)
DECLARE_TERMINATE_FUNCTION(dynfix)

#undef DECLARE_TERMINATE_FUNCTION

template<typename T> inline void Zero(forward_node<T> &node)
{
	node <<= blocks::Constant<T>(0);
}

template<typename T> inline void Zero(forward_bus<T> &bus)
{
	bus <<= blocks::RepeatedConstant<T>(0, bus.width());
}

// Disconnects the node to prevent accidental use of the node variable
template<typename T> inline void Disconnect(node<T> &theNode)
{
	using TypedNodeT = node<T>;
	theNode = TypedNodeT();
}

// Sets the bus to contain one unconnected element to avoid accidental use of the bus variable
template<typename T> inline void Disconnect(bus<T> &theBus)
{
	using TypedNodeT = node<T>;
	using TypedBusT = bus<T>;
	theBus = TypedBusT(TypedNodeT());
}


}
}
