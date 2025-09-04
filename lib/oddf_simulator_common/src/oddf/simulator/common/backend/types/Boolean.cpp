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

#include <oddf/simulator/common/backend/types/Boolean.h>

#include <oddf/utility/BooleanSupport.h>

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::types {

size_t Boolean::GetDataSize(design::NodeType const &nodeType)
{
	if (nodeType.GetTypeId() != design::NodeType::BOOLEAN)
		throw Exception(ExceptionCode::InvalidArgument);

	return GetDataSize();
}

size_t Boolean::GetDataSize() noexcept
{
	return sizeof(ValueType);
}

size_t Boolean::GetValueSize(design::NodeType const &nodeType)
{
	if (nodeType.GetTypeId() != design::NodeType::BOOLEAN)
		throw Exception(ExceptionCode::InvalidArgument);

	return GetValueSize();
}

size_t Boolean::GetValueSize() noexcept
{
	return 1;
}

bool Boolean::CheckDataIntegrity(void const *buffer, size_t bufferSize, design::NodeType const &nodeType) noexcept
{
	if (nodeType.GetTypeId() != design::NodeType::BOOLEAN)
		return false;

	return utility::BooleanCheckIntegrity(buffer, bufferSize);
}

bool Boolean::FixDataIntegrity(void *buffer, size_t bufferSize, design::NodeType const &nodeType)
{
	if (nodeType.GetTypeId() != design::NodeType::BOOLEAN)
		throw Exception(ExceptionCode::InvalidArgument);

	return utility::BooleanFixIntegrity(buffer, bufferSize);
}

bool Boolean::CheckIntegrity(design::NodeType const &nodeType) const noexcept
{
	return CheckDataIntegrity(GetData(), GetDataSize(), nodeType);
}

bool Boolean::CheckIntegrity() const noexcept
{
	return CheckDataIntegrity(GetData(), GetDataSize(), design::NodeType::Boolean());
}

void Boolean::CopyData(void *dest, size_t destSize, void const *src, size_t srcSize, design::NodeType const &nodeType)
{
	if (nodeType.GetTypeId() != design::NodeType::BOOLEAN)
		throw Exception(ExceptionCode::InvalidArgument);

	utility::BooleanCopy(dest, destSize, src, srcSize);
}

void Boolean::CopyData(void *dest, size_t destSize, void const *src, size_t srcSize)
{
	utility::BooleanCopy(dest, destSize, src, srcSize);
}

void *Boolean::GetData() noexcept
{
	return &m_value;
}

void const *Boolean::GetData() const noexcept
{
	return &m_value;
}

} // namespace oddf::simulator::common::backend::types
