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

	NearestCast() uses nearest rounding to convert from one number type
	to another.

*/

#include "../global.h"

#include "nearest_cast.h"

namespace dfx {
namespace blocks {

//
// Conversion to bool
//

template<> bus<bool> NearestCast(bool const &, bus_access<bool> const &input, CastMode)
{
	return input;
}

// template<> bus<bool> NearestCast(bool const &, bus_access<std::int32_t> const &);
// template<> bus<bool> NearestCast(bool const &, bus_access<std::int64_t> const &);
// template<> bus<bool> NearestCast(bool const &, bus_access<double> const &);
// template<> bus<bool> NearestCast(bool const &, bus_access<dynfix> const &);


//
// Conversion to int32
//

template<> bus<std::int32_t> NearestCast(std::int32_t const &, bus_access<bool> const &input, CastMode castMode)
{
	return FloorCast<std::int32_t>(input, castMode);
}

template<> bus<std::int32_t> NearestCast(std::int32_t const &, bus_access<std::int32_t> const &input, CastMode)
{
	return input;
}

template<> bus<std::int32_t> NearestCast(std::int32_t const &, bus_access<std::int64_t> const &input, CastMode castMode)
{
	return FloorCast<std::int32_t>(input, castMode);
}

template<> bus<std::int32_t> NearestCast(std::int32_t const &, bus_access<double> const &input, CastMode castMode)
{
	return FloorCast<std::int32_t>(input + 0.5, castMode);
}

template<> bus<std::int32_t> NearestCast(std::int32_t const &, bus_access<dynfix> const &input, CastMode castMode)
{
	return ReinterpretCast<std::int32_t>(NearestCast<sfix<32>>(input, castMode));
}


//
// Conversion to int64
//

template<> bus<std::int64_t> NearestCast(std::int64_t const &, bus_access<bool> const &input, CastMode castMode)
{
	return FloorCast<std::int64_t>(input, castMode);
}

template<> bus<std::int64_t> NearestCast(std::int64_t const &, bus_access<std::int32_t> const &input, CastMode castMode)
{
	return FloorCast<std::int64_t>(input, castMode);
}

template<> bus<std::int64_t> NearestCast(std::int64_t const &, bus_access<std::int64_t> const &input, CastMode)
{
	return input;
}

template<> bus<std::int64_t> NearestCast(std::int64_t const &, bus_access<double> const &input, CastMode castMode)
{
	return FloorCast<std::int64_t>(input + 0.5, castMode);
}

template<> bus<std::int64_t> NearestCast(std::int64_t const &, bus_access<dynfix> const &input, CastMode castMode)
{
	return ReinterpretCast<std::int64_t>(NearestCast<sfix<64>>(input, castMode));
}


//
// Conversion to double
//

template<> bus<double> NearestCast(double const &, bus_access<bool> const &input, CastMode castMode)
{
	return FloorCast<double>(input, castMode);
}

template<> bus<double> NearestCast(double const &, bus_access<std::int32_t> const &input, CastMode castMode)
{
	return FloorCast<double>(input, castMode);
}

template<> bus<double> NearestCast(double const &, bus_access<std::int64_t> const &input, CastMode castMode)
{
	return FloorCast<double>(input, castMode);
}

template<> bus<double> NearestCast(double const &, bus_access<double> const &input, CastMode)
{
	return input;
}

template<> bus<double> NearestCast(double const &, bus_access<dynfix> const &input, CastMode castMode)
{
	return FloorCast<double>(input, castMode);
}


//
// Conversion to dynfix
//

/*

Not tested

template<> bus<dynfix> NearestCast(dynfix const &outputTemplate, bus_access<std::int32_t> const &input)
{
	int fraction = outputTemplate.GetFraction();

	if (fraction >= 0)
		return FloorCast(outputTemplate, input);
	else
		return FloorCast(outputTemplate, input + (1 << (-1 - fraction)));
}

template<> bus<dynfix> NearestCast(dynfix const &outputTemplate, bus_access<std::int64_t> const &input)
{
	int fraction = outputTemplate.GetFraction();

	if (fraction >= 0)
		return FloorCast(outputTemplate, input);
	else
		return FloorCast(outputTemplate, input + (1 << (-1 - fraction)));
}
*/

template<> bus<dynfix> NearestCast(dynfix const &outputTemplate, bus_access<double> const &input, CastMode castMode)
{
	return FloorCast(outputTemplate, input + std::ldexp(0.5, -outputTemplate.GetFraction()), castMode);
}

template<> bus<dynfix> NearestCast(dynfix const &outputTemplate, bus_access<dynfix> const &input, CastMode castMode)
{
	return FloorCast(outputTemplate, input + std::ldexp(0.5, -outputTemplate.GetFraction()), castMode);
}

}
}
