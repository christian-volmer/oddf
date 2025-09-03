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

#include "SimulatorComponent.h"
#include "SimulatorCore.h"
#include "SimulatorBlockBase/Internals.h"

#include <oddf/simulator/common/backend/SimulatorInstruction.h>

#include <oddf/simulator/backend/IClockable.h>

#include <oddf/Exception.h>

#include <algorithm>
#include <cassert>

namespace oddf::simulator::common::backend {

SimulatorComponent::SimulatorComponent(SimulatorCore &simulatorCore) :
	m_simulatorCore(simulatorCore),
	m_code(),
	m_invalid(true),
	m_blocks(),
	m_componentObjects()
{
}

void SimulatorComponent::EnsureValidState()
{
	m_simulatorCore.EnsureValidComponentState(*this);
}

void SimulatorComponent::InvalidateState()
{
	m_simulatorCore.InvalidateComponentState(*this);
}

void SimulatorComponent::RegisterComponentObject(Uid const &clsid, std::unique_ptr<IObject> &&object)
{
	if (m_componentObjects.find(clsid) == m_componentObjects.end()) {

		// Try if we can obtain an IClockable
		try {

			auto &clockable = object->GetInterface<simulator::backend::IClockable>();
			m_simulatorCore.RegisterClockable(clockable);
		}
		catch (Exception &e) {

			if (e.GetCode() != ExceptionCode::NoInterface)
				throw;
		}

		m_componentObjects.insert({ clsid, std::move(object) });
	}
	else
		throw Exception(ExceptionCode::Fail, "RegisterComponentObject(): an object with that class id already exists.");
}

void *SimulatorComponent::GetComponentObject(Uid const &clsid, Uid const &iid) const
{
	auto objectIt = m_componentObjects.find(clsid);

	if (objectIt == m_componentObjects.end())
		throw Exception(ExceptionCode::NoResource);
	else
		return objectIt->second.get()->GetInterface(iid);
}

void SimulatorComponent::AddBlock(SimulatorBlockBase &block)
{
	assert(!block.m_internals->m_component);
	block.m_internals->m_component = this;
	m_blocks.push_back(&block);
}

void SimulatorComponent::MoveAppendFromOther(SimulatorComponent *other)
{
	assert(other);

	for (auto *block : other->m_blocks) {

		assert(block->m_internals->m_component == other);
		block->m_internals->m_component = this;
	}

	m_blocks.splice(m_blocks.end(), other->m_blocks);

	for (auto *block : m_blocks)
		assert(block->m_internals->m_component == this);

	assert(other->IsEmpty());
}

void SimulatorComponent::MovePrependFromOther(SimulatorComponent *other)
{
	assert(other);

	for (auto *block : other->m_blocks) {

		assert(block->m_internals->m_component == other);
		block->m_internals->m_component = this;
	}

	m_blocks.splice(m_blocks.begin(), other->m_blocks);

	for (auto *block : m_blocks)
		assert(block->m_internals->m_component == this);

	assert(other->IsEmpty());
}

size_t SimulatorComponent::GetSize() const
{
	return m_blocks.size();
}

bool SimulatorComponent::IsEmpty() const
{
	return m_blocks.empty();
}

void SimulatorComponent::Execute()
{
	auto *pCurrentInstruction = reinterpret_cast<SimulatorInstruction *>(m_code.data());

	while (pCurrentInstruction) {

		pCurrentInstruction->Execute();
		pCurrentInstruction = pCurrentInstruction->m_next;
	}
}

} // namespace oddf::simulator::common::backend
