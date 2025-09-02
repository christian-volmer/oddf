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

#include "SimulatorNodeHierarchyNode.h"

#include <oddf/simulator/backend/ISimulatorNodeEnumerator.h>

#include <oddf/utility/GetInterfaceHelper.h>
#include <oddf/utility/MakeEnumerator.h>

namespace oddf::simulator::common::backend {

SimulatorCore::SimulatorNodeHierarchyNode::SimulatorNodeHierarchyNode(std::string const &name, SimulatorNodeHierarchyNode *parent) :
	m_name(name),
	m_parent(parent),
	m_children(),
	m_nodes()
{
}

std::string SimulatorCore::SimulatorNodeHierarchyNode::GetName() const
{
	return m_name;
}

bool SimulatorCore::SimulatorNodeHierarchyNode::HasData() const noexcept
{
	return !m_nodes.empty();
}

std::unique_ptr<IObject> SimulatorCore::SimulatorNodeHierarchyNode::GetData() const
{
	class SimulatorNodeEnumerator : public virtual simulator::backend::ISimulatorNodeEnumerator {

	private:

		using enumeratorT = std::unique_ptr<IEnumerator<simulator::backend::ISimulatorNode const &>>;

		enumeratorT m_nodeEnumerator;

	public:

		SimulatorNodeEnumerator(enumeratorT &&nodeEnumerator) :
			m_nodeEnumerator(std::move(nodeEnumerator))
		{
		}

		virtual simulator::backend::ISimulatorNode const &GetCurrent() const override
		{
			return m_nodeEnumerator->GetCurrent();
		}

		virtual bool MoveNext()
		{
			return m_nodeEnumerator->MoveNext();
		}

		virtual void Reset()
		{
			m_nodeEnumerator->Reset();
		}

		virtual void *GetInterface(oddf::Uid const &iid) override
		{
			return utility::GetInterfaceHelper<ISimulatorNodeEnumerator, IObject>::GetInterface(this, iid);
		}
	};

	return std::make_unique<SimulatorNodeEnumerator>(
		utility::MakeEnumerator(m_nodes.begin(), m_nodes.end(), [](auto &e) -> simulator::backend::ISimulatorNode const & { return e; }));
}

bool SimulatorCore::SimulatorNodeHierarchyNode::HasChildren() const noexcept
{
	return !m_children.empty();
}

std::unique_ptr<IEnumerator<IHierarchyNode const &>> SimulatorCore::SimulatorNodeHierarchyNode::GetChildren() const
{
	return utility::MakeEnumerator(m_children.begin(), m_children.end(), [](auto &e) -> IHierarchyNode const & { return *e; });
}

IHierarchyNode const *SimulatorCore::SimulatorNodeHierarchyNode::GetParent() const
{
	return m_parent;
}

void *SimulatorCore::SimulatorNodeHierarchyNode::GetInterface(oddf::Uid const &iid)
{
	return oddf::utility::GetInterfaceHelper<IHierarchyNode, IObject>::GetInterface(this, iid);
}

} // namespace oddf::simulator::common::backend
