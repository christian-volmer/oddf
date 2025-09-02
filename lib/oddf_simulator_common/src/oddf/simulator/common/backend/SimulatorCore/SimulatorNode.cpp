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

    Definition of class `SimulatorCore::SimulatorNode`, which implements
    the `simulator::backend::ISimulatorNode` interface.

*/

#include "SimulatorNode.h"

#include <oddf/utility/GetInterfaceHelper.h>

namespace oddf::simulator::common::backend {

SimulatorCore::SimulatorNode::SimulatorNode(std::string const &name, SimulatorBlockOutput const *output) :
	m_name(name),
	m_type(),
	m_pointer(nullptr)
{
	m_type = output->GetType();
	m_pointer = output->GetPointer<void>();
}

void *SimulatorCore::SimulatorNode::GetInterface(oddf::Uid const &iid)
{
	return oddf::utility::GetInterfaceHelper<
		simulator::backend::ISimulatorNode, IObject>::GetInterface(this, iid);
}

std::string SimulatorCore::SimulatorNode::GetName() const
{
	return m_name;
}

design::NodeType SimulatorCore::SimulatorNode::GetType() const
{
	return m_type;
}

std::unique_ptr<simulator::backend::ISimulatorNodeAccess> SimulatorCore::SimulatorNode::GetAccess() const
{
	throw Exception(ExceptionCode::NotImplemented);
}

} // namespace oddf::simulator::common::backend
