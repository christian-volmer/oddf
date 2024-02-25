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

	Example program that iterates the logistic map
	https://en.wikipedia.org/wiki/Logistic_map

*/

#include "../../lib/oddf/src/dfx.h"
#include "../../lib/verilog/verilog.h"

namespace b = dfx::blocks;

int main()
{
	using dfx::dynfix;
	using dfx::ufix;

	dfx::Design design;

	using type = ufix<34, 33>;
	dfx::forward_node<type> value;

	{
		DFX_INSTANCE("LogisticMap", "LogisticMap");

		double r = 3.8;

		value <<= b::Decide(
			!b::Delay(b::Constant(true)),
			b::Constant<type>(0.5),
			b::Delay(b::FloorCast<type>(r * value * (1 - value))));

		DFX_OUTPUT(value);
	}

	dfx::debug::Logger.Log("out", value);

	dfx::Simulator simulator(design);

	simulator.Run(50);

	dfx::debug::Logger.WriteTable(std::cout);

	dfx::generator::Generator generator(design, std::cout);
	VerilogExporter exporter(generator);

	exporter.Export(".", std::cout);

	return 0;
}
