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

    Provides the `ISimulatorNode` interface that represents a node in the
    simulator.

*/

#pragma once

#include "ISimulatorNodeAccess.h"

#include <string>
#include <memory>

namespace oddf {

namespace simulator::backend {

class ISimulatorNode : public virtual IObject {

public:

	// Returns the name of the node.
	virtual std::string GetName() const = 0;

	// Returns the type of the node.
	virtual design::NodeType GetType() const = 0;

	// Returns an `ISimulatorNodeAccess` interface to the node.
	virtual std::unique_ptr<ISimulatorNodeAccess> GetAccess() const = 0;
};

} // namespace simulator::backend

template<>
struct Iid<simulator::backend::ISimulatorNode> {

	static constexpr Uid value = { 0x950075e0, 0x7f0c, 0x4643, 0xb6, 0xcb, 0x95, 0xd6, 0x5a, 0xca, 0x61, 0xf6 };
};

} // namespace oddf
