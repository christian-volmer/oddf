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

#include <oddf/design/NodeType.h>

#include <cstdint>
#include <type_traits>

namespace oddf::simulator::common::backend::types {

struct FixedPoint {

	using ElementType = std::uint16_t;
	using IntermediateType = std::uint32_t;

	using DataType = ElementType;

	// The size of an individual element, in bytes.
	static constexpr size_t ElementSize = sizeof(ElementType);

	// The size of an individual element, in bits.
	static constexpr size_t ElementBitSize = ElementSize * 8;

	static constexpr ElementType SignedExtension = ElementType(-1);
	static constexpr ElementType SignedMinimumNegativeElement = SignedExtension - SignedExtension / 2;

	size_t m_length;
	ElementType m_elements[1];

	FixedPoint() :
		m_length(size_t(-1)), m_elements() { };

	FixedPoint(FixedPoint const &) = delete;
	void operator=(FixedPoint const &) = delete;

	static size_t GetDataSize(design::NodeType const &nodeType);
	size_t GetDataSize() const noexcept;

	static size_t GetValueSize(design::NodeType const &nodeType);

	static size_t GetElementCount(design::NodeType const &nodeType);

	static bool CheckDataIntegrity(void const *buffer, size_t bufferSize, design::NodeType const &nodeType) noexcept;
	static bool FixDataIntegrity(void *buffer, size_t bufferSize, design::NodeType const &nodeType);

	bool CheckIntegrity(design::NodeType const &nodeType) const noexcept;

	static void CopyData(void *dest, size_t destSize, void const *src, size_t srcSize, design::NodeType const &nodeType);

	void *GetData() noexcept;
	void const *GetData() const noexcept;
};

static_assert(std::is_unsigned_v<FixedPoint::ElementType>);
static_assert(std::is_unsigned_v<FixedPoint::IntermediateType>);
static_assert(sizeof(FixedPoint::IntermediateType) == 2 * sizeof(FixedPoint::ElementType));

} // namespace oddf::simulator::common::backend::types
