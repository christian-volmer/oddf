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

#include <type_traits>

namespace oddf::simulator {

template<typename integralT>
class Probe<integralT, std::enable_if_t<std::is_integral_v<integralT> && !std::is_same_v<integralT, bool>>> {

private:

	backend::IProbeAccess &m_probeAccess;
	design::NodeType const m_type;

public:

	Probe(oddf::simulator::ISimulator &simulator, std::string const &name) :
		m_probeAccess(simulator.GetSimulatorAccess().GetNamedObjectInterface<backend::IProbeAccess>(name)),
		m_type(m_probeAccess.GetType())
	{
		switch (m_type.GetTypeId()) {

			case design::NodeType::FIXED_POINT:

				if (m_type.GetFraction() > 0)
					throw Exception(ExceptionCode::Unsupported, "Reading a non-integer node through a C++ integer type is not supported.");

				if (m_type.IsSigned() && std::is_unsigned_v<integralT>)
					throw Exception(ExceptionCode::Unsupported, "Reading a signed integer node through an unsigned C++ integer type is not supported.");

				break;

			default:
				throw Exception(ExceptionCode::Unsupported, "The node type cannot be read through a C++ integer type.");
		}
	}

	integralT GetValue()
	{
		size_t const bitShift = size_t(-m_type.GetFraction());

		if (m_type.IsUnsigned()) {

			using unsignedT = std::make_unsigned_t<integralT>;

			unsignedT unsignedValue { 0 };

			// Read will throw if the value is outside the range of `unsignedT`.
			m_probeAccess.Read(&unsignedValue, sizeof(unsignedValue));

			if (unsignedValue && bitShift) {

				// Since `unsignedValue` is > 0, any bit shift larger than the word width
				// of `integralT` minus 1 will overflow.
				if (bitShift > 8 * sizeof(unsignedValue) - 1)
					throw Exception(ExceptionCode::Overflow);

				unsignedT overflow = unsignedT(1) << (8 * sizeof(unsignedValue) - bitShift);

				if (unsignedValue >= overflow)
					throw Exception(ExceptionCode::Overflow);

				unsignedT scale = unsignedT(1) << bitShift;

				unsignedValue *= scale;
			}

			if constexpr (std::is_unsigned_v<integralT>) {

				return unsignedValue;
			}
			else {

				unsignedT overflowForSigned = unsignedT(1) << (8 * sizeof(unsignedValue) - 1);

				if (unsignedValue >= overflowForSigned)
					throw Exception(ExceptionCode::Overflow);

				return integralT(unsignedValue);
			}
		}
		else {

			if constexpr (std::is_unsigned_v<integralT>) {

				// Should have been cought in constructor.
				throw Exception(ExceptionCode::Unexpected);
			}
			else {

				using signedT = integralT;

				signedT signedValue { 0 };

				// Read will throw if the value is outside the range of `signedT`.
				m_probeAccess.Read(&signedValue, sizeof(signedValue));

				if (signedValue && bitShift) {

					if (signedValue >= 0) {

						if (bitShift > 8 * sizeof(signedValue) - 2)
							throw Exception(ExceptionCode::Overflow);

						signedT overflow = signedT(1) << (8 * sizeof(signedValue) - 1 - bitShift);

						if (signedValue >= overflow)
							throw Exception(ExceptionCode::Overflow);
					}
					else {

						if (bitShift > 8 * sizeof(signedValue) - 1)
							throw Exception(ExceptionCode::Overflow);

						signedT overflow = signedT(1) << (8 * sizeof(signedValue) - 1 - bitShift);
						overflow = -overflow;

						if (signedValue < overflow)
							throw Exception(ExceptionCode::Overflow);
					}

					// `bitShift` is > 0. Subtract 1 to avoid overflow with signed `scale`.
					signedT scale = signedT(1) << (bitShift - 1);

					signedValue *= scale;
					signedValue *= 2;
				}

				return signedValue;
			}
		}
	}
};

} // namespace oddf::simulator
