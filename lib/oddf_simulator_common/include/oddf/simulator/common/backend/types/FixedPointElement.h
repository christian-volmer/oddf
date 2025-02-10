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

namespace oddf::simulator::common::backend::types {

struct FixedPointElement {

	using ElementType = std::uint8_t;

private:

	ElementType m_content;

public:

	FixedPointElement() :
		m_content() {};

	FixedPointElement(FixedPointElement const &) = delete;
	void operator=(FixedPointElement const &) = delete;

	// Returns the number of elements required to store values of the given node type.
	static size_t RequiredElementCount(design::NodeType const &nodeType);
};

} // namespace oddf::simulator::common::backend::types
