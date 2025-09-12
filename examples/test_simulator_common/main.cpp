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

#include <chrono>

namespace b = dfx::blocks;
namespace sim = oddf::simulator;

namespace oddf::simulator {

class Logger : public virtual backend::IClockable {

	struct LoggedNode {

		std::string m_path;
		std::unique_ptr<simulator::backend::ISimulatorNodeAccess> m_nodeAccess;

		LoggedNode(std::string const &path, std::unique_ptr<simulator::backend::ISimulatorNodeAccess> &&nodeAccess) :
			m_path(path), m_nodeAccess(std::move(nodeAccess))
		{
		}
	};

	backend::ISimulatorAccess &m_simulatorAccess;

	size_t m_clockCycle;
	std::vector<LoggedNode> m_loggedNodes;

	void TraverseNodeHierarchy(ResourcePath const &currentHierarchyPath, IHierarchyNode const &currentHierarchyNode)
	{
		for (auto simulatorNodes = utility::ChangeInterface<simulator::backend::INamedSimulatorNodeEnumerator>(currentHierarchyNode.GetData());
			simulatorNodes->MoveNext();) {

			auto &simulatorNode = simulatorNodes->GetCurrent();

			auto simulatorNodePath = currentHierarchyPath.Append(ResourcePath::Parse(simulatorNode.GetName()));
			m_loggedNodes.emplace_back(simulatorNodePath.ToString(), simulatorNode.GetAccess());
		}

		for (auto children = currentHierarchyNode.GetChildren(); children->MoveNext();) {

			auto const &childHierarchyNode = children->GetCurrent();
			auto childPath = currentHierarchyPath.Append(ResourcePath::Parse(childHierarchyNode.GetName()));
			TraverseNodeHierarchy(childPath, childHierarchyNode);
		}
	}

public:

	Logger(ISimulator &simulator) :
		m_simulatorAccess(simulator.GetSimulatorAccess()),
		m_clockCycle(0),
		m_loggedNodes()
	{
		m_simulatorAccess.RegisterClockable(*this);

		TraverseNodeHierarchy({}, m_simulatorAccess.GetNodeHierarchyRoot());
	}

	virtual ~Logger()
	{
		m_simulatorAccess.UnregisterClockable(*this);
	}

	virtual void Clock() override
	{
		std::cout << "cycle = " << m_clockCycle << "\n";
		for (auto const &loggedNode : m_loggedNodes) {

			std::int64_t value;

			loggedNode.m_nodeAccess->Read(&value, sizeof(value));

			std::cout << "  " << loggedNode.m_path << " = " << value << "\n";
		}
		++m_clockCycle;
	}

	virtual void *GetInterface(Uid const &iid) override
	{
		return utility::GetInterfaceHelper<IObject, IClockable>::GetInterface(this, iid);
	}
};

} // namespace oddf::simulator

int main()
{
	/*

	- Next steps
	    - Simulator options
	        - Elaboration options (split bus instances yes/no) --> see e.g., BooleanFlat::Elaborate
	        - Verbosity
	    - Reporting: general, info, warning, error
	    - Logging
	        - Busses
	        - Name aliases?
	        - Export as CSV
	        - Print as table
	        - Get as vector (or copy to std container)
	    - Boolean
	        ==, !=, decide, select, replace
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

	// Coefficients of exponent 1 .. n (the x^0 term is ignored).
	// Here: 1 + x^2 + x^5
	dfx::bus<bool> taps = b::Constant({ false, true, false, false, true });

	dfx::bus<bool> reset = b::RepeatedConstant(false, taps.width());
	reset[0] = !b::Delay(b::Constant(true));

	dfx::forward_bus<bool> state(taps.width());

	state <<= b::Or(b::Delay(dfx::join(
						b::ReductionXor(b::And(state, taps)),
						state.most())),
		reset);

	b::Probe(state[0], "out");

	b::Probe(-b::Sum(-b::Constant<dynfix>({ 123.5, 654, 222 })), "testProbe");

	sim::common::Simulator simulator;
	simulator.TranslateDesign(design);

	auto out = sim::Probe<bool>(simulator, "out");
	auto testProbe = sim::Probe<double>(simulator, "testProbe");

	size_t len = (1 << taps.width()) - 1;

	for (int j = 0; j < 3; ++j) {

		for (size_t i = 0; i < len; ++i) {

			std::cout << out.GetValue();
			simulator.Run(1);
		}
		std::cout << "\n";
	}

	std::cout << "\ntestProbe = " << testProbe.GetValue() << "\n";

	return 0;
}
