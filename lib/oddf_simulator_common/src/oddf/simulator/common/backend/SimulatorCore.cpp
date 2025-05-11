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
	m_namedNodesRoot("")
{
	RegisterDefaultBlockFactories();
}

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
	NamedNode *current = &m_namedNodesRoot;

	for (auto const &elem : path) {

		current = const_cast<NamedNode *>(&*current->m_children.insert(elem).first);
	}

	if (current->m_pointer)
		throw Exception(ExceptionCode::InvalidArgument, "RegisterNamedNode(): a node has already been registered under the given path.");

	current->m_pointer = output.GetPointer<void>();
	current->m_type = output.GetType();
}

simulator::backend::ISimulatorNodeTreeElement const &SimulatorCore::GetNamedNodesRoot() const
{
	return m_namedNodesRoot;
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
