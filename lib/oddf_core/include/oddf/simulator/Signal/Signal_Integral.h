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

	backend::ISignalAccess *m_signalAccess;
	design::NodeType m_type;

	void Initialise(oddf::simulator::ISimulator &simulator, ResourcePath const &resourcePath)
	{
		m_signalAccess = &simulator.GetSimulatorAccess().GetNamedObjectInterface<backend::ISignalAccess>(":signals" + ResourcePath::Parse("/").Append(resourcePath).ToString());
		m_type = m_signalAccess->GetType();

		switch (m_type.GetTypeId()) {

			case design::NodeType::FIXED_POINT:

				if (m_type.GetFraction() != 0)
					throw Exception(ExceptionCode::Unsupported);
				break;

			default:
				throw Exception(ExceptionCode::Unsupported);
		}
	}

public:

	Signal(oddf::simulator::ISimulator &simulator, ResourcePath const &resourcePath) :
		m_signalAccess(),
		m_type()
	{
		Initialise(simulator, resourcePath);
	}

	Signal(oddf::simulator::ISimulator &simulator, std::string const &resourcePath) :
		m_signalAccess(),
		m_type()
	{
		Initialise(simulator, ResourcePath::Parse(resourcePath));
	}

	Signal(Signal const &) = default;
	Signal &operator=(Signal const &) = default;

	void SetValue(integralT const value)
	{
		// TODO: most types of overflow will be caught inside
		// the Write() function below. But I think not all.
		// Needs to be double-checked.

		m_signalAccess->Write(&value, sizeof(value));
	}
};

} // namespace oddf::simulator
