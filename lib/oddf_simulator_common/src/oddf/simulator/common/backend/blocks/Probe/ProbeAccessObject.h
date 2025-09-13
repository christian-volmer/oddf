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

#include <oddf/design/NodeType.h>

#include <oddf/utility/GetInterfaceHelper.h>

namespace oddf::simulator::common::backend::blocks {

class ProbeAccessObject : public virtual simulator::backend::IProbeAccess {

	std::unique_ptr<simulator::backend::ISimulatorNodeAccess> m_nodeAccess;

public:

	~ProbeAccessObject() = default;

	ProbeAccessObject(ProbeAccessObject const &) = delete;
	void operator=(ProbeAccessObject const &) = delete;

	ProbeAccessObject(std::unique_ptr<simulator::backend::ISimulatorNodeAccess> &&nodeAccess) :
		m_nodeAccess(std::move(nodeAccess))
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
		return m_nodeAccess->GetType();
	}

	virtual size_t GetSize() const noexcept override
	{
		return m_nodeAccess->GetSize();
	}

	virtual void Read(void *buffer, size_t bufferSize) const override
	{
		m_nodeAccess->EnsureValid();
		m_nodeAccess->Read(buffer, bufferSize);
	}
};

} // namespace oddf::simulator::common::backend::blocks
