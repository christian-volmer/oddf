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

	BitCompose() converts a bus of bits to a number.

*/

#pragma once

namespace dfx {
namespace blocks {

template<typename T> node<T> inline BitCompose(T const &, bus_access<bool> const &)
{
	throw design_error("dfx::blocks::BitCompose: the given output type is not supported.");
}

template<> node<std::int32_t> BitCompose(std::int32_t const &, bus_access<bool> const &);
template<> node<std::int64_t> BitCompose(std::int64_t const &, bus_access<bool> const &);
template<> node<dynfix> BitCompose(dynfix const &, bus_access<bool> const &);

template<typename T> node<T> inline BitCompose(node<T> const &templateNode, bus_access<bool> const &bits)
{
	return BitCompose<T>(templateNode.GetDriver()->value, bits);
}

template<typename T> node<typename types::TypeTraits<T>::internalType> inline BitCompose(bus_access<bool> const &bits)
{
	return BitCompose<typename types::TypeTraits<T>::internalType>(T(), bits);
}

}
}
