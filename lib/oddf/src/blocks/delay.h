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

	Delay() inserts a pipeline delay (flip-flop) into the design.

*/

#pragma once

namespace dfx {
namespace blocks {

#define DECLARE_DELAY_FUNCTION(_type_) \
	node<_type_> Delay(node<_type_> const &input, std::string const &tag = ""); \
	bus<_type_> Delay(bus_access<_type_> const &inputBus, std::string const &tag = ""); \
 \
	node<_type_> inline MultiDelay(node<_type_> const &input, int count, std::string const &tag = "") \
	{ \
		node<_type_> output = input; \
		while (count-- > 0) { \
			output = Delay(output, tag); \
		} \
		return output; \
	} \
 \
	bus<_type_> inline MultiDelay(bus_access<_type_> const &inputBus, int count, std::string const &tag = "") \
	{ \
		bus<_type_> outputBus = inputBus; \
		while (count-- > 0) { \
 \
			outputBus = Delay(outputBus, tag); \
		} \
		return outputBus; \
	}

DECLARE_DELAY_FUNCTION(bool)
DECLARE_DELAY_FUNCTION(double)
DECLARE_DELAY_FUNCTION(std::int32_t)
DECLARE_DELAY_FUNCTION(std::int64_t)
DECLARE_DELAY_FUNCTION(dynfix)

#undef DECLARE_DELAY_FUNCTION

}
}

std::string inline _Delay_AutomaticName(std::string id)
{
	std::transform(id.begin(), id.end(), id.begin(), [](char c) {
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_'))
			return c;
		else if (c == '.')
			return '_';
		else
			throw dfx::design_error("DFX_DELAY and DFX_MULTI_DELAY macros should be used with a simple node or bus variable.");
	});

	return id;
}

#define DFX_DELAY(input) dfx::blocks::Delay((input), _Delay_AutomaticName(#input))
#define DFX_MULTI_DELAY(input, count) dfx::blocks::MultiDelay((input), (count), _Delay_AutomaticName(#input))
