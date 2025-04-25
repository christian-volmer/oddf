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

    <no description>

*/

#include "../../lib/oddf/src/dfx.h"

#include <oddf/SimulatorCommon.h>
#include <oddf/simulator/Signal.h>
#include <oddf/simulator/Probe.h>

namespace b = dfx::blocks;
namespace sim = oddf::simulator;

int main()
{
	using dfx::dynfix;
	using dfx::sfix;
	using dfx::ufix;

	//
	// Design
	//

	dfx::Design design;

	// b::Probe(b::Delay(-b::Signal(oddf::design::NodeType::FixedPoint(true, 8, 0))));

	dfx::forward_node<ufix<16>> x;

	b::Probe(x, "super_probe");

	auto incr = b::Signal(oddf::design::NodeType::FixedPoint(true, 8, 0), "duper_signal");

	x <<= b::Delay(b::FloorCast<ufix<16>>(x + incr));

	/*	x[0] = b::Constant<dynfix>(123);
	    x[1] = b::Constant<dynfix>(-200);
	    x[2] = b::Constant<dynfix>(+1024);
	    x[3] = -b::Signal(oddf::design::NodeType::FixedPoint(true, 7, 0));

	    b::Probe(b::Sum(x));*/

	/*	b::Probe(-b::Constant<dynfix>(100));
	    b::Signal(oddf::design::NodeType::FixedPoint(true, 8, 0));*/

	/*
	bool temp = 0;
	b::Probe(!b::Delay(!b::Signal(&temp)));
	*/

	//
	// Simulation
	//

	sim::common::Simulator simulator;

	simulator.TranslateDesign(design);

	/*

	Die Benahmsung sollten wir �berpr�fen, ggf. statt std::string einen ResourcePath verwenden.
	// -SimulatorObject mit Pfad "/xxx/yyy/probes:UserId".

	*/

	auto myProbe = sim::Probe<int>(simulator, "/super_probe");
	auto mySignal = sim::Signal<int>(simulator, "/duper_signal");

	std::cout << "myprobe = " << myProbe.GetValue() << "\n";
	std::cout << "run\n";
	simulator.Run(1);

	std::cout << "myprobe = " << myProbe.GetValue() << "\n";
	std::cout << "signal = -1\n";
	mySignal.SetValue(11);
	std::cout << "myprobe = " << myProbe.GetValue() << "\n";

	for (int i = 0; i < 20; ++i) {

		simulator.Run(1);
		std::cout << "myprobe = " << myProbe.GetValue() << "\n";
	}

	return 0;
}
