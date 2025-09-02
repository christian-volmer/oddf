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
#include <oddf/simulator/backend/ISimulatorNodeEnumerator.h>

#include <oddf/utility/GetInterfaceHelper.h>

namespace b = dfx::blocks;
namespace sim = oddf::simulator;

namespace oddf::simulator {

class Logger : public virtual backend::IClockable {

	struct Node {

		bool m_valid;
		std::map<std::string, Node> m_children;

		Node() :
			m_valid(false),
			m_children()
		{
		}
	};

	backend::ISimulatorAccess &m_simulatorAccess;
	IHierarchyNode const &m_nodesRoot;

public:

	Logger(ISimulator &simulator) :
		m_simulatorAccess(simulator.GetSimulatorAccess()),
		m_nodesRoot(m_simulatorAccess.GetNodeHierarchyRoot())
	{
		m_simulatorAccess.RegisterClockable(*this);
	}

	virtual ~Logger()
	{
		m_simulatorAccess.UnregisterClockable(*this);
	}

	virtual void Clock() override
	{
	}

	virtual void *GetInterface(Uid const &iid) override
	{
		return utility::GetInterfaceHelper<IObject, IClockable>::GetInterface(this, iid);
	}

	void Dump(std::string const &path, IHierarchyNode const &node)
	{
		std::string subPath = path + node.GetName() + "/";
		std::cout << subPath << "\n";

		{

			auto *enObj = node.GetData().release();
			std::unique_ptr<simulator::backend::ISimulatorNodeEnumerator> nodes(&enObj->GetInterface<simulator::backend::ISimulatorNodeEnumerator>());

			for (; nodes->MoveNext();) {

				auto &current = nodes->GetCurrent();

				std::cout << "  " << current.GetName() << ": " << current.GetType().ToString() << "\n";
			}
		}

		for (auto children = node.GetChildren(); children->MoveNext();)
			Dump(subPath, children->GetCurrent());
	}

	void Dump()
	{
		Dump("", m_nodesRoot);
	}
};

} // namespace oddf::simulator

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

	b::Probe(x, "probe");

	auto incr = b::Signal(oddf::design::NodeType::FixedPoint(true, 8, 0), "duper_signal");

	{
		DFX_INSTANCE("instance1", "my_module");

		b::Probe(x, "probe");
		b::Probe(x + 3, "my_probe");
	}

	/*

	Nächste Schritte:

	    - Über generische hierarchische Namespaces im Simulator nachdenken.
	       - für Instanzen
	       - für named nodes (Label, Name, Input, Output)
	       - für Probes und Signals
	       - für Modelle (Sin, oder komplexere Dinge)

	    - Generische Möglichkeit, Namespaces zu durchlaufen
	    - An jedem Knoten über GetInterface ein entsprechendes Interface abrufen
	    - Wir haben momentan die "Named Simulator Objects", reicht das nicht auch
	      für alles was nicht einem Knoten entspricht?
	        - Vielleicht für spätere Erweiterungen zusätzlich die generischen Namespaces. Man weiß ja nie?
	    - ACHTUNG: Wir dürfen den Simulator aber nicht mit dem Design verwechseln!

	    - Implementieren
	      - Logging
	      - Busse
	        - Evtl. PRBS mit Bus-Bools, Bus-AND und Reduction-XOR?

	*/

	b::Probe(x + 12345678, "probe2");

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

	sim::Logger logger(simulator);
	logger.Dump();

	return 0;

	auto myProbe = sim::Probe<int>(simulator, "/instance1/my_probe");
	auto mySignal = sim::Signal<int>(simulator, "/duper_signal");

	std::cout << "myprobe = " << myProbe.GetValue() << "\n";
	std::cout << "run\n";
	simulator.Run(1);

	std::cout << "myprobe = " << myProbe.GetValue() << "\n";
	std::cout << "signal = -1\n";
	mySignal.SetValue(11);
	std::cout << "myprobe = " << myProbe.GetValue() << "\n";

	for (int i = 0; i < 20; ++i) {

		simulator.Run(2);
		std::cout << "myprobe = " << myProbe.GetValue() << "\n";
	}

	return 0;
}
