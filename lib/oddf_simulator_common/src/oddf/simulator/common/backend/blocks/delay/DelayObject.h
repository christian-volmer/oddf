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

#include <oddf/simulator/common/backend/Types.h>
#include <oddf/simulator/common/backend/ISimulatorComponent.h>

#include <oddf/simulator/backend/IClockable.h>

#include <oddf/utility/GetInterfaceHelper.h>
#include <oddf/Clsid.h>

#include <list>
#include <cassert>

namespace oddf {

namespace simulator::common::backend::blocks {

class DelayStateBase {

protected:

	DelayStateBase() = default;

public:

	virtual ~DelayStateBase() = default;

	DelayStateBase(DelayStateBase const &) = delete;
	void operator=(DelayStateBase const &) = delete;

	virtual void Clock() = 0;
};

template<typename simulatorT>
class DelayState : public DelayStateBase {

private:

	using dataT = typename simulatorT::DataType;

	design::NodeType m_nodeType;
	void const *m_source;
	size_t m_dataSize;
	std::unique_ptr<unsigned char[]> m_state;

	virtual void Clock() override
	{
		memcpy(m_state.get(), m_source, m_dataSize);

		// This is an internal data consistency check, which should never fire.
		if (!simulatorT::CheckDataIntegrity(m_state.get(), m_dataSize, m_nodeType))
			throw Exception(ExceptionCode::Unexpected);
	}

public:

	DelayState(design::NodeType const &nodeType) :
		m_nodeType(nodeType),
		m_source(),
		m_dataSize(simulatorT::GetDataSize(nodeType)),
		m_state()
	{
		m_state.reset(new unsigned char[m_dataSize] {});
	}

	DelayState(DelayState const &) = delete;
	void operator=(DelayState const &) = delete;

	dataT const &ReferenceToCurrent()
	{
		return *reinterpret_cast<dataT const *>(m_state.get());
	}

	void SetSource(simulatorT const *source)
	{
		m_source = source->GetData();
	}
};

class DelayObject : public virtual simulator::backend::IClockable {

private:

	ISimulatorComponent &m_component;

	std::list<std::unique_ptr<DelayStateBase>> m_states;

public:

	DelayObject(ISimulatorComponent &component) :
		m_component(component),
		m_states()
	{
	}

	template<typename T>
	DelayState<T> *AddState(design::NodeType const &nodeType)
	{
		auto state = std::make_unique<DelayState<T>>(nodeType);
		auto *ptr = state.get();
		m_states.emplace_back(std::move(state));
		return ptr;
	}

	//
	// IObject
	//

	virtual void *GetInterface(Uid const &iid) override
	{
		return utility::GetInterfaceHelper<IObject, IClockable>::GetInterface(this, iid);
	}

	//
	// IClockable
	//

	virtual void Clock() override
	{
		for (auto &state : m_states)
			state->Clock();

		m_component.InvalidateState();
	}
};

} // namespace simulator::common::backend::blocks

template<>
struct Clsid<simulator::common::backend::blocks::DelayObject> {

	static constexpr Uid value = { 0x17fb33b1, 0x928f, 0x44af, 0xb9, 0x30, 0xff, 0xec, 0xb8, 0x44, 0x1f, 0xb2 };
};

} // namespace oddf
