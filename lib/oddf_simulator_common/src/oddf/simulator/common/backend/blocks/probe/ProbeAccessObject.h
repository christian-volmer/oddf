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

#include <oddf/utility/GetInterfaceHelper.h>

#include <oddf/design/NodeType.h>

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

} // namespace oddf::simulator::common::backend::blocks
