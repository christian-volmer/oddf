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

#include "FixedPoint.h"
#include <oddf/design/NodeType.h>

namespace oddf::simulator::common::backend::types {

// Returns `true` if the internal representation of the fixed-
// point number pointed to by `elements` is consistent. This
// concerns the consistency of potentially unused bits in the most significant element.
bool CheckFixedPointRepresentation(FixedPoint::ElementType const *elements, size_t elementCount, design::NodeType const &nodeType);

// Returns `true` if the internal representation of the fixed-
// point number `fixedPoint` is consistent with the given `nodeType`.
// The check concerns the number of elements and the consistency of
// potentially unused bits in the most significant element.
bool CheckFixedPointRepresentation(FixedPoint const &fixedPoint, design::NodeType const &nodeType);

} // namespace oddf::simulator::common::backend::types
