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

    Example of implementing a counter.

*/

#include "../../lib/oddf/src/dfx.h"

#include <oddf/Simulator.h>
#include <oddf/utility/ContainerViews.h>
#include <set>

namespace b = dfx::blocks;

int main()
{
	using dfx::dynfix;
	using dfx::ufix;

	dfx::Design design;

#if 0

	using type = ufix<16>;
	dfx::forward_node<type> current_value;

	// The starting value of our counter
	type starting_value = 0;

	// The increment of our counter
	type increment = 1;

	type stopping_value = 10;

	// The following creates a reset pulse at the beginning of the simulation
	dfx::node<bool> reset = !b::Delay(b::Constant(true));

	{
		DFX_INSTANCE("I_counter", "Counter");

		DFX_INPUT(reset);

		current_value <<= b::Decide(
			reset, starting_value,
			b::Delay(b::Decide(
				current_value < stopping_value, b::FloorCast<type>(current_value + increment),
				current_value)));

		DFX_OUTPUT(current_value);
	}

	dfx::debug::Logger.Log("reset", reset);
	dfx::debug::Logger.Log("out", current_value);

#endif

	b::Terminate(b::Delay(b::Delay(b::Constant(true))));

	/*
	    dfx::Simulator simulator(design);
	    simulator.Run(15);
	    dfx::debug::Logger.WriteTable(std::cout);
	*/

	oddf::Simulator simulator2;

	simulator2.TranslateDesign(design);

	return 0;
}
