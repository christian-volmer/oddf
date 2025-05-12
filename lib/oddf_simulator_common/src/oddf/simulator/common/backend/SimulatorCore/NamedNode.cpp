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

#include "NamedNode.h"

#include <oddf/utility/GetInterfaceHelper.h>

namespace oddf::simulator::common::backend {

SimulatorCore::NamedNode::NamedNode(std::string const &name) :
	m_name(name),
	m_type(),
	m_pointer(),
	m_children()
{
}

std::string SimulatorCore::NamedNode::GetName() const
{
	return m_name;
}

bool SimulatorCore::NamedNode::IsNode() const
{
	return m_pointer;
}

design::NodeType SimulatorCore::NamedNode::GetType() const
{
	if (m_pointer)
		return m_type;
	else
		throw Exception(ExceptionCode::IllegalMethodCall, "This node tree element has no associated node.");
}

void SimulatorCore::NamedNode::Read(void * /*buffer */, size_t /* count */) const
{
	if (m_pointer)
		throw Exception(ExceptionCode::NotImplemented);
	else
		throw Exception(ExceptionCode::IllegalMethodCall, "This node tree element has no associated node.");
}

utility::CollectionView<simulator::backend::ISimulatorNodeTreeElement const &> SimulatorCore::NamedNode::GetChildren() const
{
	return utility::MakeCollectionView<ISimulatorNodeTreeElement const &>(m_children);
}

void *SimulatorCore::NamedNode::GetInterface(oddf::Uid const &iid)
{
	return oddf::utility::GetInterfaceHelper<
		simulator::backend::ISimulatorNodeTreeElement,
		IObject>::GetInterface(this, iid);
}

} // namespace oddf::simulator::common::backend
