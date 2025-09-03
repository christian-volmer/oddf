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

#include <oddf/utility/CopyInteger.h>
#include <oddf/utility/GetInterfaceHelper.h>

#include <oddf/Exception.h>

#include <cassert>

namespace oddf::simulator::common::backend {

class SimulatorBlockOutput::SimulatorNode::SimulatorNodeAccessFixedPoint
	: public virtual simulator::backend::ISimulatorNodeAccess {
private:

	ISimulatorComponent *m_component;
	void const *m_pointer;
	design::NodeType m_type;

public:

	SimulatorNodeAccessFixedPoint(ISimulatorComponent *component, void const *pointer, design::NodeType type) :
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
		return types::GetRequiredByteSize(m_type);
	}

	virtual void EnsureValid() override
	{
		m_component->EnsureValidState();
	}

	virtual void Read(void * /* buffer */, size_t /* bufferSize */) const override
	{
		throw Exception(ExceptionCode::NotImplemented);

		/*
		    TODO
		     - CheckFixedPointRepresentation() sollte intern werden und jeden Typen überprüfen
		       --> CheckInternalRepresentation?

		     - CopySignedInteger und alle Verwandten sollen, wenn möglich keine void-Zeiger
		       akzeptieren, sondern mit den Typen aus dem `types` namespace arbeiten. Man
		       brancht dann funktionen für beide Richtungen (cf. für Signal und für Probe)

		        - Dann muss man auch nicht immer die Größe angeben.

		     - GetPointer<void>() entfernen?

		     - Geht das alles, oder haben wir hier noch void-Zeiger herumfahren, die sich nicht
		       vermeiden lassen?
		*/

		/*assert(types::CheckFixedPointRepresentation(*m_probedOutputPointer, m_nodeType));

		if (m_nodeType.IsSigned())
		    utility::CopySignedInteger(buffer, bufferSize, m_pointer, types::GetStoredByteSize(m_nodeType));
		else
		    utility::CopyUnsignedInteger(buffer, bufferSize, m_pointer, types::GetStoredByteSize(m_nodeType));*/
	}

	virtual void *GetInterface(oddf::Uid const &iid) override
	{
		return oddf::utility::GetInterfaceHelper<
			simulator::backend::ISimulatorNodeAccess,
			IObject>::GetInterface(this, iid);
	}
};

} // namespace oddf::simulator::common::backend
