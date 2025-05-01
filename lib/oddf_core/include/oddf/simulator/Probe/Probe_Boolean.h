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

#include "../backend/IProbeAccess.h"

namespace oddf::simulator {

template<>
class Probe<bool> {

private:

	backend::IProbeAccess *m_probeAccess;

	void Initialise(oddf::simulator::ISimulator &simulator, ResourcePath const &resourcePath)
	{
		m_probeAccess = &simulator.GetSimulatorAccess().GetNamedObjectInterface<backend::IProbeAccess>(":probes" + ResourcePath::Parse("/").Append(resourcePath).ToString());

		if (m_probeAccess->GetType().GetTypeId() != design::NodeType::BOOLEAN)
			throw Exception(ExceptionCode::Unsupported);
	}

public:

	Probe(oddf::simulator::ISimulator &simulator, ResourcePath const &resourcePath) :
		m_probeAccess()
	{
		Initialise(simulator, resourcePath);
	}

	Probe(oddf::simulator::ISimulator &simulator, std::string const &resourcePath) :
		m_probeAccess()
	{
		Initialise(simulator, ResourcePath::Parse(resourcePath));
	}

	Probe(Probe const &) = default;
	Probe &operator=(Probe const &) = default;

	bool GetValue()
	{
		std::uint8_t value;
		m_probeAccess->Read(&value, sizeof(value));
		return value != 0;
	}
};

} // namespace oddf::simulator
