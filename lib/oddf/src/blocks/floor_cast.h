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

	FloorCast() uses rounding towards negative infinity to convert from
	one number type to another.

*/

#pragma once

namespace dfx {

namespace blocks {

enum class CastMode {

	WrapAround = 0,
	Saturate = 1
};


template<typename toT, typename fromT> bus<typename types::TypeTraits<toT>::internalType> inline FloorCast(toT const &, bus_access<fromT> const &, CastMode = CastMode::WrapAround)
{
	throw design_error("dfx::blocks::FloorCast: cast between the given types is not supported.");
}

template<> bus<bool> FloorCast(bool const &, bus_access<bool> const &, CastMode);
// template<> bus<bool> FloorCast(bool const &, bus_access<std::int32_t> const &);
// template<> bus<bool> FloorCast(bool const &, bus_access<std::int64_t> const &);
// template<> bus<bool> FloorCast(bool const &, bus_access<double> const &);
// template<> bus<bool> FloorCast(bool const &, bus_access<dynfix> const &);

template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<bool> const &, CastMode);
template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<std::int32_t> const &, CastMode);
template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<std::int64_t> const &, CastMode);
template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<double> const &, CastMode);
template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<dynfix> const &, CastMode);

template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<bool> const &, CastMode);
template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<std::int32_t> const &, CastMode);
template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<std::int64_t> const &, CastMode);
template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<double> const &, CastMode);
template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<dynfix> const &, CastMode);

template<> bus<double> FloorCast(double const &, bus_access<bool> const &, CastMode);
template<> bus<double> FloorCast(double const &, bus_access<std::int32_t> const &, CastMode);
template<> bus<double> FloorCast(double const &, bus_access<std::int64_t> const &, CastMode);
template<> bus<double> FloorCast(double const &, bus_access<double> const &, CastMode);
template<> bus<double> FloorCast(double const &, bus_access<dynfix> const &, CastMode);

//template<> bus<dynfix> FloorCast(dynfix const &, bus_access<bool> const &);
template<> bus<dynfix> FloorCast(dynfix const &, bus_access<std::int32_t> const &, CastMode);
template<> bus<dynfix> FloorCast(dynfix const &, bus_access<std::int64_t> const &, CastMode);
template<> bus<dynfix> FloorCast(dynfix const &, bus_access<double> const &, CastMode);
template<> bus<dynfix> FloorCast(dynfix const &, bus_access<dynfix> const &, CastMode);


template<typename toT, typename fromT> node<typename types::TypeTraits<toT>::internalType> inline FloorCast(toT const &templateValue, node<fromT> const &node, CastMode castMode = CastMode::WrapAround)
{
	return FloorCast(templateValue, bus<fromT>(node), castMode).first();
}

template<typename toT, typename fromT> node<toT> inline FloorCast(node<toT> const &templateNode, node<fromT> const &node, CastMode castMode = CastMode::WrapAround)
{
	return FloorCast<toT, fromT>(templateNode.GetDriver()->value, bus<fromT>(node), castMode).first();
}

template<typename toT, typename fromT> bus<toT> inline FloorCast(node<toT> const &templateNode, bus_access<fromT> const &bus, CastMode castMode = CastMode::WrapAround)
{
	return FloorCast<toT, fromT>(templateNode.GetDriver()->value, bus, castMode);
}

template<typename toT, typename fromT> node<typename types::TypeTraits<toT>::internalType> inline FloorCast(node<fromT> const &node, CastMode castMode = CastMode::WrapAround)
{
	return FloorCast<typename types::TypeTraits<toT>::internalType, fromT>(toT(), bus<fromT>(node), castMode).first();
}

template<typename toT, typename fromT> bus<typename types::TypeTraits<toT>::internalType> inline FloorCast(bus_access<fromT> const &bus, CastMode castMode = CastMode::WrapAround)
{
	return FloorCast<typename types::TypeTraits<toT>::internalType, fromT>(toT(), bus, castMode);
}

}
}
