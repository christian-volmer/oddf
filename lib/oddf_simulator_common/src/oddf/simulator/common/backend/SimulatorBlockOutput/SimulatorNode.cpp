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

    Defines class `SimulatorBlockOutput::SimulatorNode`.

*/

#include "SimulatorNode.h"
#include "SimulatorNode/SimulatorNodeAccessBool.h"
#include "SimulatorNode/SimulatorNodeAccessFixedPoint.h"

#include <oddf/utility/GetInterfaceHelper.h>

#include <cassert>

namespace oddf::simulator::common::backend {

SimulatorBlockOutput::SimulatorNode::SimulatorNode(ISimulatorComponent *component, design::NodeType type, void const *pointer) :
	m_component(component),
	m_type(type),
	m_pointer(pointer)
{
	assert(component && pointer);
	assert(type.IsDefined());
}

design::NodeType SimulatorBlockOutput::SimulatorNode::GetType() const
{
	return m_type;
}

std::unique_ptr<simulator::backend::ISimulatorNodeAccess> SimulatorBlockOutput::SimulatorNode::GetAccess() const
{
	switch (m_type.GetTypeId()) {

		case design::NodeType::BOOLEAN:
			return std::make_unique<SimulatorNodeAccessBool>(m_component,
				reinterpret_cast<types::Boolean const *>(m_pointer));

		case design::NodeType::FIXED_POINT:
			return std::make_unique<SimulatorNodeAccessFixedPoint>(m_component,
				reinterpret_cast<types::FixedPoint const *>(m_pointer), m_type);

		default:
			break;
	}

	throw Exception(ExceptionCode::NotImplemented);
}

void *SimulatorBlockOutput::SimulatorNode::GetInterface(oddf::Uid const &iid)
{
	return oddf::utility::GetInterfaceHelper<
		simulator::backend::ISimulatorNode,
		IObject>::GetInterface(this, iid);
}

} // namespace oddf::simulator::common::backend
