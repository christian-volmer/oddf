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
#include <oddf/utility/ChangeInterface.h>

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

		for (auto nodes = utility::ChangeInterface<simulator::backend::INamedSimulatorNodeEnumerator>(node.GetData());
			nodes->MoveNext();) {

			auto &current = nodes->GetCurrent();

			std::cout << "  " << current.GetName() << ": " << current.GetType().ToString() << "\n";
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
	/*

	- Next steps
	    - Comment new type support functions
	    - Logging
	    - Busses
	        - Evtl. PRBS mit Bus-Bools, Bus-AND und Reduction-XOR?
	        - Full Boolean support (NOT, AND, OR, XOR, Reduction, ==, !=)
	    - Assertions?
	    - Models?
	    - Fixed-Point arithmetic
	        Helper-block to expand inputs to common type (helps with plus, <=>, decide)
	    - Double
	    - Machine integer

	*/

	using dfx::dynfix;
	using dfx::sfix;
	using dfx::ufix;

	//
	// Design
	//

	dfx::Design design;

	dfx::node<dynfix> value = b::Signal(oddf::design::NodeType::FixedPoint(false, 19, 0), "ValueSignal");

	bool _tempBit = false;
	dfx::node<bool> bit = b::Signal(&_tempBit, "BoolSignal");

	{
		DFX_INSTANCE("instance1", "my_module");

		b::Probe(b::Delay(100 - value), "ValueProbe");
		b::Probe(b::Delay(bit), "BoolProbe");

		{
			DFX_INSTANCE("sub_instance1", "my_module2");
			b::Probe(!b::Constant(true), "ConstantBoolProbe");
		}
	}

	//
	// Simulation
	//

	sim::common::Simulator simulator;

	simulator.TranslateDesign(design);

	std::cout << " --- logger.Dump() --- \n";

	sim::Logger logger(simulator);
	logger.Dump();

	std::cout << "\n";

	// return 0;

	auto valueSignal = sim::Signal<int>(simulator, "/ValueSignal");
	auto boolSignal = sim::Signal<bool>(simulator, "/BoolSignal");

	auto valueProbe = sim::Probe<int>(simulator, "/instance1/ValueProbe");
	auto boolProbe = sim::Probe<bool>(simulator, "/instance1/BoolProbe");
	auto constantBoolProbe = sim::Probe<bool>(simulator, "/instance1/sub_instance1/ConstantBoolProbe");

	std::cout << "ValueProbe         = " << valueProbe.GetValue() << "\n";
	std::cout << "BoolProbe          = " << boolProbe.GetValue() << "\n";
	std::cout << "ConstantBoolProbe  = " << constantBoolProbe.GetValue() << "\n";
	std::cout << "\n";

	std::cout << "Setting ValueSignal = 123 and BoolSignal = true.\n";

	valueSignal.SetValue(123);
	boolSignal.SetValue(true);

	std::cout << "ValueProbe = " << valueProbe.GetValue() << "\n";
	std::cout << "BoolProbe  = " << boolProbe.GetValue() << "\n";
	std::cout << "\n";

	std::cout << "Toggling clock.\n";

	simulator.Run(1);

	std::cout << "ValueProbe = " << valueProbe.GetValue() << "\n";
	std::cout << "BoolProbe  = " << boolProbe.GetValue() << "\n";

	return 0;
}
