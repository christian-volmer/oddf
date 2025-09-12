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

#include <oddf/ResourcePath.h>

#include <type_traits>
#include <cmath>
#include <cstdint>

namespace oddf::simulator {

template<typename floatT>
class Probe<floatT, std::enable_if_t<std::is_floating_point_v<floatT>>> {

private:

	backend::IProbeAccess *m_probeAccess;
	design::NodeType m_type;

	void Initialise(oddf::simulator::ISimulator &simulator, ResourcePath const &resourcePath)
	{
		m_probeAccess = &simulator.GetSimulatorAccess().GetNamedObjectInterface<backend::IProbeAccess>(":probes" + ResourcePath::Parse("/").Append(resourcePath).ToString());
		m_type = m_probeAccess->GetType();

		switch (m_type.GetTypeId()) {

			case design::NodeType::FIXED_POINT:
				break;

			default:
				throw Exception(ExceptionCode::Unsupported, "The node type cannot be read through a C++ integer type.");
		}
	}

public:

	Probe(oddf::simulator::ISimulator &simulator, ResourcePath const &resourcePath) :
		m_probeAccess(),
		m_type()
	{
		Initialise(simulator, resourcePath);
	}

	Probe(oddf::simulator::ISimulator &simulator, std::string const &resourcePath) :
		m_probeAccess(),
		m_type()
	{
		Initialise(simulator, ResourcePath::Parse(resourcePath));
	}

	Probe(Probe const &) = default;
	Probe &operator=(Probe const &) = default;

	floatT GetValue()
	{
		auto fraction = static_cast<floatT>(m_type.GetFraction());
		floatT scaling = std::exp2(-fraction);

		if (m_type.IsUnsigned()) {

			std::uintmax_t unsignedValue;
			m_probeAccess->Read(&unsignedValue, sizeof(unsignedValue));
			return scaling * unsignedValue;
		}
		else {

			std::intmax_t signedValue;
			m_probeAccess->Read(&signedValue, sizeof(signedValue));
			return scaling * signedValue;
		}
	}
};

} // namespace oddf::simulator
