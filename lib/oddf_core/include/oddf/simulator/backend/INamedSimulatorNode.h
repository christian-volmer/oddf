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

    Provides the `INamedSimulatorNode` interface that represents a node in the
    simulator with information about name in the design.

*/

#pragma once

#include "ISimulatorNode.h"

namespace oddf {

namespace simulator::backend {

class INamedSimulatorNode : public virtual ISimulatorNode {

public:

	// Returns the name of the node.
	virtual std::string GetName() const = 0;
};

} // namespace simulator::backend

template<>
struct Iid<simulator::backend::INamedSimulatorNode> {

	static constexpr Uid value = { 0xd4bbf740, 0x4dfc, 0x46c4, 0xbd, 0x1c, 0x7e, 0x33, 0xf9, 0x5f, 0xe3, 0xff };
};

} // namespace oddf
