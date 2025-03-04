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

#include "../backend/ISignalAccess.h"

#include <type_traits>

namespace oddf::simulator {

template<typename integralT>
class Signal<integralT, std::enable_if_t<std::is_integral_v<integralT> && !std::is_same_v<integralT, bool>>> {

private:

	backend::ISignalAccess &m_signalAccess;
	design::NodeType const m_type;

public:

	Signal(oddf::simulator::ISimulator &simulator, std::string const &name) :
		m_signalAccess(simulator.GetSimulatorAccess().GetNamedObjectInterface<backend::ISignalAccess>(name)),
		m_type(m_signalAccess.GetType())
	{
		switch (m_type.GetTypeId()) {

			case design::NodeType::FIXED_POINT:

				if (m_type.GetFraction() != 0)
					throw Exception(ExceptionCode::Unsupported);
				break;

			default:
				throw Exception(ExceptionCode::Unsupported);
		}
	}

	void SetValue(integralT const value)
	{
		/*
		    TODO catch overflow based on the actual type of the underlying node
		    Do it here? Do it inside SignalAccessObject? Do it inside CopySignedInteger/CopyUnsignedInteger?
		*/

		m_signalAccess.Write(&value, sizeof(value));
	}
};

} // namespace oddf::simulator
