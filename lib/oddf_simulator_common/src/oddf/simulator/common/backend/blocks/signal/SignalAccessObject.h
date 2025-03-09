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

#include <oddf/simulator/backend/ISignalAccess.h>

#include <oddf/simulator/common/backend/ISimulatorComponent.h>
#include <oddf/simulator/common/backend/Types.h>
#include <oddf/simulator/common/backend/types/CheckFixedPointRepresentation.h>

#include <oddf/utility/GetInterfaceHelper.h>
#include <oddf/utility/CopyBoolean.h>
#include <oddf/utility/CopyInteger.h>

#include <memory>

namespace oddf::simulator::common::backend::blocks {

template<typename simulatorT>
class SignalAccessObject : public virtual simulator::backend::ISignalAccess {

private:

	using SimulatorType = simulatorT;

	ISimulatorComponent &m_component;
	design::NodeType m_nodeType;

	std::unique_ptr<SimulatorType[]> m_value;

public:

	SignalAccessObject(ISimulatorComponent &component, design::NodeType const &nodeType);

	SimulatorType const &GetSource() const
	{
		return m_value[0];
	}

	virtual void *GetInterface(Uid const &iid) override
	{
		return utility::GetInterfaceHelper<
			simulator::backend::ISignalAccess,
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

	virtual void Write(void const *buffer, size_t count) override;
};

//
// Implementation for types::Boolean
//

template<>
inline SignalAccessObject<types::Boolean>::SignalAccessObject(ISimulatorComponent &component, design::NodeType const &nodeType) :
	m_component(component),
	m_nodeType(nodeType),
	m_value(new SimulatorType[1] {})
{
}

template<>
inline void SignalAccessObject<types::Boolean>::Write(void const *buffer, size_t count)
{
	utility::CopyBoolean(m_value.get(), types::GetStoredByteSize(m_nodeType), buffer, count);
	m_component.InvalidateState();
}

//
// Implementation for types::FixedPointElement
//

template<>
inline SignalAccessObject<types::FixedPointElement>::SignalAccessObject(ISimulatorComponent &component, design::NodeType const &nodeType) :
	m_component(component),
	m_nodeType(nodeType),
	m_value(new SimulatorType[SimulatorType::RequiredElementCount(nodeType)] {})
{
}

template<>
inline void SignalAccessObject<types::FixedPointElement>::Write(void const *buffer, size_t count)
{
	if (m_nodeType.IsSigned())
		utility::CopySignedInteger(m_value.get(), types::GetStoredByteSize(m_nodeType), buffer, count);
	else
		utility::CopyUnsignedInteger(m_value.get(), types::GetStoredByteSize(m_nodeType), buffer, count);

	if (!types::CheckFixedPointRepresentation(m_value.get(), m_nodeType))
		throw Exception(ExceptionCode::Overflow);

	m_component.InvalidateState();
}

} // namespace oddf::simulator::common::backend::blocks
