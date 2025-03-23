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

#include <oddf/simulator/common/backend/types/GetStoredByteSize.h>

#include <oddf/simulator/common/backend/types/Boolean.h>
#include <oddf/simulator/common/backend/types/FixedPoint.h>

namespace oddf::simulator::common::backend::types {

size_t GetStoredByteSize(design::NodeType const &nodeType)
{
	switch (nodeType.GetTypeId()) {

		case design::NodeType::BOOLEAN:
			return sizeof(types::Boolean);

		case design::NodeType::FIXED_POINT:
			return sizeof(types::FixedPoint::ElementType) * types::FixedPoint::RequiredElementCount(nodeType);

		case design::NodeType::INTEGER:
		case design::NodeType::BIT_VECTOR:
		case design::NodeType::REAL:
			throw Exception(ExceptionCode::NotImplemented);

		default:
			throw Exception(ExceptionCode::Unexpected);
	}
}

} // namespace oddf::simulator::common::backend::types
