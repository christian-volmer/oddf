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

#pragma once

#include <oddf/simulator/backend/IProbeAccess.h>

#include <oddf/simulator/common/backend/ISimulatorComponent.h>
#include <oddf/simulator/common/backend/SimulatorBlockOutput.h>
#include <oddf/simulator/common/backend/types/CheckFixedPointRepresentation.h>

#include <oddf/design/NodeType.h>

#include <oddf/utility/CopyBoolean.h>
#include <oddf/utility/CopyInteger.h>
#include <oddf/utility/GetInterfaceHelper.h>

#include <cassert>

namespace oddf::simulator::common::backend::blocks {

template<typename simulatorT>
class ProbeAccessObject : public virtual simulator::backend::IProbeAccess {

	ISimulatorComponent &m_component;
	design::NodeType m_nodeType;
	simulatorT const *m_probedOutputPointer;

public:

	~ProbeAccessObject() = default;

	ProbeAccessObject(ProbeAccessObject<simulatorT> const &) = delete;
	void operator=(ProbeAccessObject<simulatorT> const &) = delete;

	ProbeAccessObject(ISimulatorComponent &component, SimulatorBlockOutput const &driver) :
		m_component(component),
		m_nodeType(driver.GetType()),
		m_probedOutputPointer(driver.GetPointer<simulatorT>())
	{
	}

	ProbeAccessObject(ISimulatorComponent &component, SimulatorBlockOutput const &driver, size_t elementCount) :
		m_component(component),
		m_nodeType(driver.GetType()),
		m_probedOutputPointer(driver.GetPointer<simulatorT>(elementCount))
	{
	}

	virtual void *GetInterface(Uid const &iid) override
	{
		return utility::GetInterfaceHelper<
			simulator::backend::IProbeAccess,
			IObject>::GetInterface(this, iid);
	}

	virtual design::NodeType GetType() const noexcept override
	{
		return m_nodeType;
	}

	virtual size_t GetSize() const noexcept override
	{
		return types::GetRequiredByteSize(m_nodeType);
	}

	virtual void Read(void *buffer, size_t count) const override;
};

//
// Implementation for types::Boolean
//

template<>
inline void ProbeAccessObject<types::Boolean>::Read(void *buffer, size_t count) const
{
	m_component.EnsureValidState();
	utility::CopyBoolean(buffer, count, m_probedOutputPointer, types::GetStoredByteSize(m_nodeType));
}

//
// Implementation for types::FixedPointElement
//

template<>
inline void ProbeAccessObject<types::FixedPointElement>::Read(void *buffer, size_t count) const
{
	m_component.EnsureValidState();

	assert(types::CheckFixedPointRepresentation(m_probedOutputPointer, m_nodeType));

	if (m_nodeType.IsSigned())
		utility::CopySignedInteger(buffer, count, m_probedOutputPointer, types::GetStoredByteSize(m_nodeType));
	else
		utility::CopyUnsignedInteger(buffer, count, m_probedOutputPointer, types::GetStoredByteSize(m_nodeType));
}

} // namespace oddf::simulator::common::backend::blocks
