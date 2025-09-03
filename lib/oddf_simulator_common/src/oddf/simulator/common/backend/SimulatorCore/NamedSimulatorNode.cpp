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

    Definition of class `SimulatorCore::NamedSimulatorNode`, which implements
    the `simulator::backend::INamedSimulatorNode` interface.

*/

#include "NamedSimulatorNode.h"

#include <oddf/utility/GetInterfaceHelper.h>

namespace oddf::simulator::common::backend {

SimulatorCore::NamedSimulatorNode::NamedSimulatorNode(std::string const &name, std::unique_ptr<simulator::backend::ISimulatorNode> &&simulatorNode) :
	m_name(name),
	m_simulatorNode(std::move(simulatorNode))
{
}

void *SimulatorCore::NamedSimulatorNode::GetInterface(oddf::Uid const &iid)
{
	return oddf::utility::GetInterfaceHelper<
		simulator::backend::INamedSimulatorNode,
		simulator::backend::ISimulatorNode,
		IObject>::GetInterface(this, iid);
}

std::string SimulatorCore::NamedSimulatorNode::GetName() const
{
	return m_name;
}

design::NodeType SimulatorCore::NamedSimulatorNode::GetType() const
{
	return m_simulatorNode->GetType();
}

std::unique_ptr<simulator::backend::ISimulatorNodeAccess> SimulatorCore::NamedSimulatorNode::GetAccess() const
{
	return m_simulatorNode->GetAccess();
}

} // namespace oddf::simulator::common::backend
