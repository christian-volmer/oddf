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
	backend::ISimulatorNodeTreeElement const &m_nodesRoot;

public:

	Logger(ISimulator &simulator) :
		m_simulatorAccess(simulator.GetSimulatorAccess()),
		m_nodesRoot(m_simulatorAccess.GetNamedNodesRoot())
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

	void Dump(std::string const &path, backend::ISimulatorNodeTreeElement const &node)
	{
		std::string subPath = path + node.GetName();

		if (node.IsNode())
			std::cout << subPath << " : " << node.GetType().ToString() << "\n";

		subPath += "/";

		auto children = node.GetChildren();
		for (auto enumerator = children->GetEnumerator(); enumerator->MoveNext();)
			Dump(subPath, enumerator->GetCurrent());
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

	// Hier gehts weiter:

	/* NamedNode und ISimulatorNodeTreeElement

	Das mit dem ListView und CollectionView funktioniert nicht zusammen
	mit ISimulatorNodeTreeElement, jedenfalls nicht im ganz allgemeinen Fall.
	Wir sollten Interfaces draus machen die �ber einen unique_ptr zur�ckgegeben
	werden. Intern benutzen die ja ohnehin einen Zeiger auf ihre Implementierung,
	evtl. kann man das elegant in einem l�sen.

	Probe sollte intern �ber NamedNode den gleichen Mechanismus wie NamedNode benutzen?
	Sollten eine Funktion GetNamedNode() im Simulator haben.


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
