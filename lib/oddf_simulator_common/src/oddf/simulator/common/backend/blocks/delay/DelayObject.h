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
#include <oddf/simulator/common/backend/IClockable.h>
#include <oddf/simulator/common/backend/ISimulatorComponent.h>
#include <oddf/simulator/common/backend/types/CheckFixedPointRepresentation.h>
#include <oddf/utility/GetInterfaceHelper.h>
#include <oddf/Clsid.h>

#include <list>
#include <cassert>

namespace oddf {

namespace simulator::common::backend::blocks {

class DelayStateBase {
public:

	DelayStateBase() = default;
	virtual ~DelayStateBase() = default;

	DelayStateBase(DelayStateBase const &) = delete;
	void operator=(DelayStateBase const &) = delete;

	virtual void Clock() = 0;
};

template<typename T, typename = void>
class DelayState;

template<typename T>
class DelayState<T, std::void_t<typename T::ValueType>> : public DelayStateBase {

private:

	T const *m_pSource;
	T m_current;

	virtual void Clock() override
	{
		m_current.m_value = m_pSource->m_value;
	}

public:

	DelayState(design::NodeType const &) :
		m_pSource(), m_current() { }

	DelayState(DelayState const &) = delete;
	void operator=(DelayState const &) = delete;

	T const &ReferenceToCurrent()
	{
		return m_current;
	}

	void SetSource(T const *pSource)
	{
		m_pSource = pSource;
	}
};

template<typename T>
class DelayState<T, std::void_t<typename T::ElementType>> : public DelayStateBase {

private:

	size_t m_byteCount;
	T const *m_pSource;
	std::unique_ptr<T[]> m_current;

#ifndef NDEBUG

	design::NodeType m_nodeType;

	void InternalCheck() const
	{
		if constexpr (std::is_same_v<T, types::FixedPointElement>)
			assert(types::CheckFixedPointRepresentation(m_pSource, m_nodeType));
	}

#else

	void InternalCheck() const
	{
	}

#endif

	virtual void Clock() override
	{
		InternalCheck();
		memcpy(static_cast<void *>(&m_current[0]), m_pSource, m_byteCount);
	}

public:

	DelayState(design::NodeType const &nodeType) :
		m_byteCount(T::RequiredElementCount(nodeType) * sizeof(T)),
		m_pSource(),
		m_current(new T[T::RequiredElementCount(nodeType)] {})
#ifndef NDEBUG
		,
		m_nodeType(nodeType)
#endif
	{
		(void)nodeType;
	}

	DelayState(DelayState const &) = delete;
	void operator=(DelayState const &) = delete;

	T const &ReferenceToCurrent()
	{
		return m_current[0];
	}

	void SetSource(T const *pSource)
	{
		m_pSource = pSource;
	}
};

class DelayObject : public virtual IClockable {

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
