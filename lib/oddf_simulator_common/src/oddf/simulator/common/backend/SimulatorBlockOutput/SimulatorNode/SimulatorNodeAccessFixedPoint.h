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

#include "../SimulatorNode.h"

#include <oddf/utility/GetInterfaceHelper.h>
#include <oddf/utility/IntegerSupport.h>

#include <cassert>

namespace oddf::simulator::common::backend {

class SimulatorBlockOutput::SimulatorNode::SimulatorNodeAccessFixedPoint
	: public virtual simulator::backend::ISimulatorNodeAccess {
private:

	ISimulatorComponent *m_component;
	types::FixedPoint const *m_pointer;
	design::NodeType m_type;

public:

	SimulatorNodeAccessFixedPoint(ISimulatorComponent *component, types::FixedPoint const *pointer, design::NodeType type) :
		m_component(component),
		m_pointer(pointer),
		m_type(type)
	{
		assert(component && pointer);
		assert(type.GetTypeId() == design::NodeType::FIXED_POINT);
	}

	~SimulatorNodeAccessFixedPoint() = default;

	SimulatorNodeAccessFixedPoint(SimulatorNodeAccessFixedPoint const &) = delete;
	void operator=(SimulatorNodeAccessFixedPoint const &) = delete;

	virtual design::NodeType GetType() const override
	{
		return m_type;
	}

	virtual size_t GetSize() const noexcept override
	{
		return m_pointer->GetValueSize(m_type);
	}

	virtual void EnsureValid() override
	{
		m_component->EnsureValidState();
	}

	virtual void Read(void *buffer, size_t bufferSize) const override
	{
		// This is an internal data consistency check, which should never fire.
		if (!m_pointer->CheckIntegrity(m_type))
			throw Exception(ExceptionCode::Unexpected);

		utility::IntegerCopy(buffer, bufferSize, m_pointer->GetData(), m_pointer->GetDataSize(),
			m_type.GetWordWidth(), m_type.IsSigned());
	}

	virtual void *GetInterface(oddf::Uid const &iid) override
	{
		return oddf::utility::GetInterfaceHelper<
			simulator::backend::ISimulatorNodeAccess,
			IObject>::GetInterface(this, iid);
	}
};

} // namespace oddf::simulator::common::backend
