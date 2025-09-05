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

	// The value used for sign extension in a two's complement representation.
	static constexpr ElementType SignedExtension = ElementType(-1);

	// A two's complement number is negative if the most significant element is
	// larger than or equal to this value.
	static constexpr ElementType SignedMinimumNegativeElement = SignedExtension - SignedExtension / 2;

	size_t m_length;
	ElementType m_elements[1];

	FixedPoint() :
		m_length(size_t(-1)), m_elements() { };

	FixedPoint(FixedPoint const &) = delete;
	void operator=(FixedPoint const &) = delete;

	// Returns the number of bytes used internally by the simulator to represent a
	// fixed-point number of the given type. Throws if `nodeType` is not
	// `FixedPoint`.
	static size_t GetDataSize(design::NodeType const &nodeType);

	// Returns the number of bytes used internally by this instance to
	// represent a fixed-point number.
	size_t GetDataSize() const noexcept;

	// Returns the minimum number of bytes required to fully represent a fixed-
	// point number of the given type. Throws if `nodeType` is not `FixedPoint`.
	static size_t GetValueSize(design::NodeType const &nodeType);

	// Returns the number of elements used internally by the simulator to
	// represent a fixed-point number of the given type. Throws if `nodeType` is
	// not `FixedPoint`.
	static size_t GetElementCount(design::NodeType const &nodeType);

	// Calls `utility::IntegerCheckIntegrity()` on the buffer and returns the
	// result. Returns `false` if `nodeType` is not `FixedPoint`.
	static bool CheckDataIntegrity(void const *buffer, size_t bufferSize, design::NodeType const &nodeType) noexcept;

	// Calls `utility::IntegerFixIntegrity()` on the buffer and returns the
	// result. Throws if `nodeType` is not `FixedPoint`.
	static bool FixDataIntegrity(void *buffer, size_t bufferSize, design::NodeType const &nodeType);

	// Calls `CheckDataIntegrity()` on the stored value and returns the result.
	// Returns `false` if `nodeType` is not `Boolean`.
	bool CheckIntegrity(design::NodeType const &nodeType) const noexcept;

	// Returns a void pointer to the stored value, which is a block of length
	// `GetDataSize()` bytes.
	void *GetData() noexcept;

	// Returns a constant void pointer to the stored value, which is a block of
	// length `GetDataSize()` bytes.
	void const *GetData() const noexcept;
};

static_assert(std::is_unsigned_v<FixedPoint::ElementType>);
static_assert(std::is_unsigned_v<FixedPoint::IntermediateType>);
static_assert(sizeof(FixedPoint::IntermediateType) == 2 * sizeof(FixedPoint::ElementType));

} // namespace oddf::simulator::common::backend::types
