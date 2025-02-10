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

#include <oddf/design/NodeType.h>

#include <oddf/Exception.h>
#include <cassert>

namespace oddf::design {

NodeType::NodeType(TypeId type, int param1 /* = 0 */, int param2 /* = 0 */) :
	m_type(type),
	m_param1(param1),
	m_param2(param2)
{
}

NodeType::NodeType() :
	m_type(UNDEFINED),
	m_param1(0),
	m_param2(0)
{
}

NodeType NodeType::Undefined() noexcept
{
	return NodeType();
}

NodeType NodeType::Boolean() noexcept
{
	return NodeType(BOOLEAN);
}

NodeType NodeType::Integer() noexcept
{
	return NodeType(INTEGER);
}

NodeType NodeType::Real() noexcept
{
	return NodeType(REAL);
}

NodeType NodeType::BitVector(int size)
{
	if (size < 0)
		throw Exception(ExceptionCode::InvalidArgument, "NodeType::BitVector(): parameter 'size' must not be negative.");

	return NodeType(BIT_VECTOR, size);
}

NodeType NodeType::FixedPoint(bool signedness, int size, int fraction)
{
	if (size < 0)
		throw Exception(ExceptionCode::InvalidArgument, "NodeType::FixedPoint(): parameter 'size' must not be negative.");

	return NodeType(FIXED_POINT, signedness ? -size : size, size != 0 ? fraction : 0);
}

std::string NodeType::ToString() const
{
	switch (m_type) {

		case UNDEFINED:
			return "<undefined>";

		case BOOLEAN:
			return "bool";

		case INTEGER:
			return "int";

		case REAL:
			return "double";

		case BIT_VECTOR:
			return "bitvector<" + std::to_string(m_param1) + ">";

		case FIXED_POINT: {

			std::string s = m_param1 < 0 ? "sfix<" : "ufix<";
			s += std::to_string(m_param1);
			if (m_param2 != 0)
				s += ", " + std::to_string(m_param2);
			return s + ">";
		}

		default:
			assert(false);
			return "<error>";
	}
}

NodeType::TypeId NodeType::GetTypeId() const noexcept
{
	return m_type;
}

int NodeType::GetWordWidth() const noexcept
{
	if (m_param1 >= 0)
		return m_param1;
	else
		return -m_param1;
}

bool NodeType::IsSigned() const noexcept
{
	return m_param1 < 0;
}

bool NodeType::IsUnsigned() const noexcept
{
	return !IsSigned();
}

int NodeType::GetFraction() const noexcept
{
	return m_param2;
}

bool NodeType::IsDefined() const noexcept
{
	return m_type != UNDEFINED;
}

bool NodeType::operator==(NodeType const &other) const noexcept
{
	switch (m_type) {

		case UNDEFINED:
		case BOOLEAN:
		case INTEGER:
		case REAL:
			return other.m_type == m_type;

		case BIT_VECTOR:
			return other.m_type == BIT_VECTOR && other.m_param1 == m_param1;

		case FIXED_POINT:
			return other.m_type == FIXED_POINT && other.m_param1 == m_param1 && other.m_param2 == m_param2;

		default:
			assert(false);
			return other.m_type == m_type;
	}
}

bool NodeType::operator!=(NodeType const &other) const noexcept
{
	return !(*this == other);
}

} // namespace oddf::design
