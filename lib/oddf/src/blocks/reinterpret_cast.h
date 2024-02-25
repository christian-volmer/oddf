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

	ReinterpretCast() converts one number type to another based on the
	number's binary representation.

*/

#pragma once

namespace dfx {
namespace blocks {

template<typename toT, typename fromT> bus<toT> inline ReinterpretCast(toT const &, bus_access<fromT> const &)
{
	throw design_error("dfx::blocks::ReinterpretCast: cast between the given types is not supported.");
}

template<> bus<bool> ReinterpretCast(bool const &, bus_access<bool> const &);
template<> bus<bool> ReinterpretCast(bool const &, bus_access<std::int32_t> const &);
template<> bus<bool> ReinterpretCast(bool const &, bus_access<std::int64_t> const &);
// template<> bus<bool> ReinterpretCast(bool const &, bus_access<double> const &);
 template<> bus<bool> ReinterpretCast(bool const &, bus_access<dynfix> const &);

template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &, bus_access<bool> const &);
template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &, bus_access<std::int32_t> const &);
template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &, bus_access<std::int64_t> const &);
//template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &, bus_access<double> const &);
template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &, bus_access<dynfix> const &);

template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &, bus_access<bool> const &);
template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &, bus_access<std::int32_t> const &);
template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &, bus_access<std::int64_t> const &);
template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &, bus_access<double> const &);
template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &, bus_access<dynfix> const &);

//template<> bus<double> ReinterpretCast(double const &, bus_access<bool> const &);
//template<> bus<double> ReinterpretCast(double const &, bus_access<std::int32_t> const &);
template<> bus<double> ReinterpretCast(double const &, bus_access<std::int64_t> const &);
template<> bus<double> ReinterpretCast(double const &, bus_access<double> const &);
template<> bus<double> ReinterpretCast(double const &, bus_access<dynfix> const &);

template<> bus<dynfix> ReinterpretCast(dynfix const &, bus_access<bool> const &);
template<> bus<dynfix> ReinterpretCast(dynfix const &, bus_access<std::int32_t> const &);
template<> bus<dynfix> ReinterpretCast(dynfix const &, bus_access<std::int64_t> const &);
template<> bus<dynfix> ReinterpretCast(dynfix const &, bus_access<double> const &);
template<> bus<dynfix> ReinterpretCast(dynfix const &, bus_access<dynfix> const &);


template<typename toT, typename fromT> node<typename types::TypeTraits<toT>::internalType> inline ReinterpretCast(toT const &templateValue, node<fromT> const &node)
{
	return ReinterpretCast(templateValue, bus<fromT>(node)).first();
}

template<typename toT, typename fromT> node<toT> inline ReinterpretCast(node<toT> const &templateNode, node<fromT> const &node)
{
	return ReinterpretCast<toT, fromT>(templateNode.GetDriver()->value, bus<fromT>(node)).first();
}

template<typename toT, typename fromT> node<typename types::TypeTraits<toT>::internalType> inline ReinterpretCast(node<fromT> const &node)
{
	return ReinterpretCast<typename types::TypeTraits<toT>::internalType, fromT>(toT(), bus<fromT>(node)).first();
}

template<typename toT, typename fromT> bus<typename types::TypeTraits<toT>::internalType> inline ReinterpretCast(bus_access<fromT> const &bus)
{
	return ReinterpretCast<typename types::TypeTraits<toT>::internalType, fromT>(toT(), bus);
}

}
}
