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

struct Boolean {

	using ValueType = std::uint8_t;

	using DataType = ValueType;

	ValueType m_value;

	Boolean() :
		m_value() { };

	Boolean(Boolean const &) = default;
	void operator=(Boolean const &) = delete;

	// Returns the number of bytes used internally by the simulator to represent a
	// Boolean value. Throws if `nodeType` is not `Boolean`.
	static size_t GetDataSize(design::NodeType const &nodeType);

	// Returns the number of bytes used internally by the simulator to represent a
	// Boolean value.
	static size_t GetDataSize() noexcept;

	// Returns the number 1, which is the minimum number of bytes required to
	// fully represent a Boolean value. Throws if `nodeType` is not `Boolean`.
	static size_t GetValueSize(design::NodeType const &nodeType);

	// Returns the number 1, which is the minimum number of bytes required to
	// fully represent a Boolean value.
	static size_t GetValueSize() noexcept;

	// Calls `utility::BooleanCheckIntegrity()` on the buffer and returns the
	// result. Returns `false` if `nodeType` is not `Boolean`.
	static bool CheckDataIntegrity(void const *buffer, size_t bufferSize, design::NodeType const &nodeType) noexcept;

	// Calls `utility::BooleanFixIntegrity()` on the buffer and returns the
	// result. Throws if `nodeType` is not `Boolean`.
	static bool FixDataIntegrity(void *buffer, size_t bufferSize, design::NodeType const &nodeType);

	// Calls `CheckDataIntegrity()` on the stored value and returns the result.
	// Returns `false` if `nodeType` is not `Boolean`.
	bool CheckIntegrity(design::NodeType const &nodeType) const noexcept;

	// Calls `CheckDataIntegrity()` on the stored value and returns the result.
	bool CheckIntegrity() const noexcept;

	// Returns a void pointer to the stored value, which is a block of length
	// `GetDataSize()` bytes.
	void *GetData() noexcept;

	// Returns a constant void pointer to the stored value, which is a block of
	// length `GetDataSize()` bytes.
	void const *GetData() const noexcept;
};

static_assert(std::is_unsigned_v<Boolean::ValueType>);

} // namespace oddf::simulator::common::backend::types
