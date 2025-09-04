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

#include <oddf/utility/GetInterfaceHelper.h>
#include <oddf/utility/BooleanSupport.h>
#include <oddf/utility/IntegerSupport.h>

#include <memory>

namespace oddf::simulator::common::backend::blocks {

template<typename simulatorT>
class SignalAccessObject : public virtual simulator::backend::ISignalAccess {

private:

	using dataT = typename simulatorT::DataType;

	ISimulatorComponent &m_component;
	design::NodeType m_nodeType;

	std::unique_ptr<unsigned char[]> m_value;

public:

	SignalAccessObject(ISimulatorComponent &component, design::NodeType const &nodeType) :
		m_component(component),
		m_nodeType(nodeType),
		m_value()
	{
		m_value.reset(new unsigned char[simulatorT::GetDataSize(nodeType)] {});
	}

	dataT const &GetSource() const
	{
		return *reinterpret_cast<dataT const *>(m_value.get());
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
		return simulatorT::GetValueSize(m_nodeType);
	}

	virtual void Write(void const *buffer, size_t bufferSize) override
	{
		auto *data = m_value.get();
		auto dataSize = simulatorT::GetDataSize(m_nodeType);

		try {

			simulatorT::CopyData(data, dataSize, buffer, bufferSize, m_nodeType);

			if (!simulatorT::FixDataIntegrity(data, dataSize, m_nodeType))
				throw Exception(ExceptionCode::Overflow);

			m_component.InvalidateState();
		}
		catch (...) {

			m_component.InvalidateState();
			throw;
		}
	}
};

} // namespace oddf::simulator::common::backend::blocks
