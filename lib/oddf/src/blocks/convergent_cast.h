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

	ConvergentCast() uses convergent rounding to convert from one number
	type to another.

*/

#pragma once

#include "floor_cast.h"

namespace dfx {
namespace blocks {

template<typename toT, typename fromT> bus<typename types::TypeTraits<toT>::internalType> inline ConvergentCast(toT const &, bus_access<fromT> const &, CastMode = CastMode::WrapAround)
{
	throw design_error("dfx::blocks::ConvergentCast: cast between the given types is not supported.");
}

// template<> bus<bool> ConvergentCast(bool const &, bus_access<bool> const &);
// template<> bus<bool> ConvergentCast(bool const &, bus_access<std::int32_t> const &);
// template<> bus<bool> ConvergentCast(bool const &, bus_access<std::int64_t> const &);
// template<> bus<bool> ConvergentCast(bool const &, bus_access<double> const &);
// template<> bus<bool> ConvergentCast(bool const &, bus_access<dynfix> const &);

// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<bool> const &);
// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<std::int32_t> const &);
// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<std::int64_t> const &);
// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<double> const &);
// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<dynfix> const &);

// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<bool> const &);
// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<std::int32_t> const &);
// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<std::int64_t> const &);
// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<double> const &);
// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<dynfix> const &);

// template<> bus<double> ConvergentCast(double const &, bus_access<bool> const &);
// template<> bus<double> ConvergentCast(double const &, bus_access<std::int32_t> const &);
// template<> bus<double> ConvergentCast(double const &, bus_access<std::int64_t> const &);
template<> bus<double> ConvergentCast(double const &, bus_access<double> const &, CastMode);
// template<> bus<double> ConvergentCast(double const &, bus_access<dynfix> const &);

// template<> bus<dynfix> ConvergentCast(dynfix const &, bus_access<bool> const &);
// template<> bus<dynfix> ConvergentCast(dynfix const &, bus_access<std::int32_t> const &);
// template<> bus<dynfix> ConvergentCast(dynfix const &, bus_access<std::int64_t> const &);
// template<> bus<dynfix> ConvergentCast(dynfix const &, bus_access<double> const &);
template<> bus<dynfix> ConvergentCast(dynfix const &, bus_access<dynfix> const &, CastMode);


template<typename toT, typename fromT> node<typename types::TypeTraits<toT>::internalType> inline ConvergentCast(toT const &templateValue, node<fromT> const &node, CastMode castMode = CastMode::WrapAround)
{
	return ConvergentCast(templateValue, bus<fromT>(node), castMode).first();
}

template<typename toT, typename fromT> node<toT> inline ConvergentCast(node<toT> const &templateNode, node<fromT> const &node, CastMode castMode = CastMode::WrapAround)
{
	return ConvergentCast<toT, fromT>(templateNode.GetDriver()->value, bus<fromT>(node), castMode).first();
}

template<typename toT, typename fromT> bus<toT> inline ConvergentCast(node<toT> const &templateNode, bus_access<fromT> const &bus, CastMode castMode = CastMode::WrapAround)
{
	return ConvergentCast<toT, fromT>(templateNode.GetDriver()->value, bus, castMode);
}

template<typename toT, typename fromT> node<typename types::TypeTraits<toT>::internalType> inline ConvergentCast(node<fromT> const &node, CastMode castMode = CastMode::WrapAround)
{
	return ConvergentCast<typename types::TypeTraits<toT>::internalType, fromT>(toT(), bus<fromT>(node), castMode).first();
}

template<typename toT, typename fromT> bus<typename types::TypeTraits<toT>::internalType> inline ConvergentCast(bus_access<fromT> const &bus, CastMode castMode = CastMode::WrapAround)
{
	return ConvergentCast<typename types::TypeTraits<toT>::internalType, fromT>(toT(), bus, castMode);
}

}
}
