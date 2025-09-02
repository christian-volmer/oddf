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

#include "SimulatorCore.h"
#include "SimulatorCore/SimulatorNode.h"
#include "SimulatorCore/SimulatorNodeHierarchyNode.h"

#include <oddf/Exception.h>

#include <cassert>

namespace oddf::simulator::common::backend {

SimulatorCore::SimulatorCore() :
	m_blocks(),
	m_simulatorBlockFactories(),
	m_components(),
	m_invalidComponents(),
	m_namedSimulatorObjects(),
	m_clockables(),
	m_nodeHierarchyRoot(new SimulatorNodeHierarchyNode("", nullptr))
{
	RegisterDefaultBlockFactories();
}

// Cannot be placed in header file because the type `NamedNode` is undefined there.
SimulatorCore::~SimulatorCore() = default;

void SimulatorCore::RegisterGlobalObject(std::string name, std::unique_ptr<IObject> &&object)
{
	if (m_namedSimulatorObjects.find(name) == m_namedSimulatorObjects.end())
		m_namedSimulatorObjects.insert({ name, std::move(object) });
	else
		throw Exception(ExceptionCode::Fail, "RegisterGlobalObject(): an object with that name already exists.");
}

void SimulatorCore::RegisterNamedNode(ResourcePath const &path, SimulatorBlockOutput const &output)
{
	SimulatorNodeHierarchyNode *hierarchyParent = nullptr;
	SimulatorNodeHierarchyNode *hierarchyCurrent = m_nodeHierarchyRoot.get();

	auto pathCurrent = path.begin();
	auto pathEnd = path.end();

	if (pathCurrent == pathEnd)
		throw Exception(ExceptionCode::InvalidArgument, "Argument `path` must not be empty.");

	for (; std::next(pathCurrent) != pathEnd; ++pathCurrent) {

		auto position = hierarchyCurrent->m_children.find(*pathCurrent);

		if (position == hierarchyCurrent->m_children.end())
			position = hierarchyCurrent->m_children.insert(std::make_unique<SimulatorNodeHierarchyNode>(*pathCurrent, hierarchyParent)).first;

		hierarchyParent = hierarchyCurrent;
		hierarchyCurrent = (*position).get();
	}

	if (hierarchyCurrent->m_nodes.count(*pathCurrent))
		throw Exception(ExceptionCode::InvalidArgument, "RegisterNamedNode(): a node has already been registered under the given path.");

	hierarchyCurrent->m_nodes.insert({ *pathCurrent, &output });
}

IHierarchyNode const &SimulatorCore::GetNodeHierarchyRoot() const
{
	return *m_nodeHierarchyRoot;
}

void SimulatorCore::RegisterClockable(simulator::backend::IClockable &clockable)
{
	assert(m_clockables.find(&clockable) == m_clockables.end());
	m_clockables.insert(&clockable);
}

void SimulatorCore::UnregisterClockable(simulator::backend::IClockable &clockable)
{
	auto found = m_clockables.find(&clockable);

	if (*found == &clockable)
		m_clockables.erase(found);
	else
		throw Exception(ExceptionCode::InvalidArgument, "UnregisterClockable(): the given `clockable` has not been registered before.");
}

void *SimulatorCore::GetNamedObjectInterface(std::string const &name, Uid const &iid) const
{
	auto objectIt = m_namedSimulatorObjects.find(name);

	if (objectIt == m_namedSimulatorObjects.end())
		throw Exception(ExceptionCode::NoResource);
	else
		return objectIt->second.get()->GetInterface(iid);
}

void SimulatorCore::InvalidateComponentState(SimulatorComponent &component)
{
	if (!component.m_invalid) {

		component.m_invalid = true;
		m_invalidComponents.insert(&component);
	}
}

void SimulatorCore::EnsureValidComponentState(SimulatorComponent & /* component */)
{
	EnsureAllComponentStatesValid();
}

void SimulatorCore::EnsureAllComponentStatesValid()
{
	for (auto *invalidComponent : m_invalidComponents) {

		assert(invalidComponent->m_invalid);
		invalidComponent->Execute();
		invalidComponent->m_invalid = false;
	}

	m_invalidComponents.clear();
}

} // namespace oddf::simulator::common::backend
