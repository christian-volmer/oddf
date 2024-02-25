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

#include "../global.h"

#include "convergent_cast.h"

namespace dfx {
namespace blocks {

//
// Conversion to bool
//

// template<> bus<bool> ConvergentCast(bool const &, bus_access<bool> const &input);
// template<> bus<bool> ConvergentCast(bool const &, bus_access<std::int32_t> const &);
// template<> bus<bool> ConvergentCast(bool const &, bus_access<std::int64_t> const &);
// template<> bus<bool> ConvergentCast(bool const &, bus_access<double> const &);
// template<> bus<bool> ConvergentCast(bool const &, bus_access<dynfix> const &);


//
// Conversion to int32
//

// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<bool> const &input);
// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<std::int32_t> const &input);
// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<std::int64_t> const &input);
// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<double> const &input);
// template<> bus<std::int32_t> ConvergentCast(std::int32_t const &, bus_access<dynfix> const &input);


//
// Conversion to int64
//

// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<bool> const &input);
// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<std::int32_t> const &input);
// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<std::int64_t> const &input);
// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<double> const &input);
// template<> bus<std::int64_t> ConvergentCast(std::int64_t const &, bus_access<dynfix> const &input);


//
// Conversion to double
//

// template<> bus<double> ConvergentCast(double const &, bus_access<bool> const &input);
// template<> bus<double> ConvergentCast(double const &, bus_access<std::int32_t> const &input);
// template<> bus<double> ConvergentCast(double const &, bus_access<std::int64_t> const &input);

template<> bus<double> ConvergentCast(double const &, bus_access<double> const &input, CastMode)
{
	return input;
}

// template<> bus<double> ConvergentCast(double const &, bus_access<dynfix> const &input);


//
// Conversion to dynfix
//

// template<> bus<dynfix> ConvergentCast(dynfix const &, bus_access<bool> const &);
// template<> bus<dynfix> ConvergentCast(dynfix const &, bus_access<std::int32_t> const &input);
// template<> bus<dynfix> ConvergentCast(dynfix const &, bus_access<std::int64_t> const &input);
// template<> bus<dynfix> ConvergentCast(dynfix const &, bus_access<double> const &input);

template<> bus<dynfix> ConvergentCast(dynfix const &outputTemplate, bus_access<dynfix> const &input, CastMode castMode)
{
	int width = input.width();

	if (width == 0)
		return bus<dynfix>(0);

	int sourceFraction = input[0].GetType().GetFraction();

	for (int i = 1; i < width; ++i)
		if (input[i].GetType().GetFraction() != sourceFraction)
			throw dfx::design_error("dfx::blocks::ConvergentCast: all elements of the input bus must have the same fractional part.");

	int outputFraction = outputTemplate.GetFraction();

	int fractionDrop = sourceFraction - outputFraction;

	if (fractionDrop <= 0)
		return FloorCast(outputTemplate, input, castMode);

	dynfix decisionType(false, fractionDrop + 1, 0);
	bus<dynfix> decisionBits = ReinterpretCast(decisionType, input);

	int limit = 1 << (fractionDrop - 1);

	auto correction = ((limit < decisionBits) && (decisionBits < 2 * limit)) || (decisionBits >= 3 * limit);

	dynfix correctionType(false, 1, outputFraction);
	return FloorCast(outputTemplate, input + ReinterpretCast(correctionType, correction), castMode);
}

}
}
