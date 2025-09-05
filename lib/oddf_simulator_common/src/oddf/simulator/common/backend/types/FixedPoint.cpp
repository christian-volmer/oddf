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

#include <oddf/simulator/common/backend/types/FixedPoint.h>

#include <oddf/utility/IntegerSupport.h>

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::types {

size_t FixedPoint::GetDataSize(design::NodeType const &nodeType)
{
	return GetElementCount(nodeType) * ElementSize;
}

size_t FixedPoint::GetDataSize() const noexcept
{
	return m_length * ElementSize;
}

size_t FixedPoint::GetValueSize(design::NodeType const &nodeType)
{
	if (nodeType.GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::InvalidArgument);

	auto wordWidth = nodeType.GetWordWidth();
	return (wordWidth + 7) / 8;
}

size_t FixedPoint::GetElementCount(design::NodeType const &nodeType)
{
	if (nodeType.GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::InvalidArgument);

	auto wordWidth = nodeType.GetWordWidth();
	return (wordWidth + ElementBitSize - 1) / ElementBitSize;
}

bool FixedPoint::CheckDataIntegrity(void const *buffer, size_t bufferSize, design::NodeType const &nodeType) noexcept
{
	if (nodeType.GetTypeId() != design::NodeType::FIXED_POINT)
		return false;

	return utility::IntegerCheckIntegrity(buffer, bufferSize, nodeType.GetWordWidth(), nodeType.IsSigned());
}

bool FixedPoint::FixDataIntegrity(void *buffer, size_t bufferSize, design::NodeType const &nodeType)
{
	if (nodeType.GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::InvalidArgument);

	return utility::IntegerFixIntegrity(buffer, bufferSize, nodeType.GetWordWidth(), nodeType.IsSigned());
}

bool FixedPoint::CheckIntegrity(design::NodeType const &nodeType) const noexcept
{
	// `GetElementCount()` will throw if `nodeType` is not FixedPoint. So we do
	// data integrity check first and check `m_length` later.

	if (!CheckDataIntegrity(GetData(), GetDataSize(), nodeType))
		return false;

	return m_length == GetElementCount(nodeType);
}

void *FixedPoint::GetData() noexcept
{
	return m_elements;
}

void const *FixedPoint::GetData() const noexcept
{
	return m_elements;
}

} // namespace oddf::simulator::common::backend::types
