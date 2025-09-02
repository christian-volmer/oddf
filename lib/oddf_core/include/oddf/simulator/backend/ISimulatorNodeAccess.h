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

    Provides the `ISimulatorNodeAccess` interface for reading the value on a
    simulatornode.

*/

#pragma once

#include <oddf/IObject.h>
#include <oddf/design/NodeType.h>

namespace oddf {

namespace simulator::backend {

class ISimulatorNodeAccess : public virtual IObject {

public:

	// Returns the type of the node.
	virtual design::NodeType GetType() const = 0;

	// Returns the number of bytes required to represent the value on the node.
	virtual size_t GetSize() const noexcept = 0;

	// Copies the value on the node into `buffer`. Its size must be passed as
	// parameter `bufferSize`, which must be at least the value returned by
	// `GetSize()`. Larger buffers receive appropriate padding to keep the
	// representation meaningful (e.g., sign extension). Throws if `bufferSize`
	// is too small.
	virtual void Read(void *buffer, size_t bufferSize) const = 0;
};

} // namespace simulator::backend

template<>
struct Iid<simulator::backend::ISimulatorNodeAccess> {

	static constexpr Uid value = { 0x3ca2e391, 0xabaf, 0x493f, 0xbe, 0x92, 0x17, 0x1f, 0x24, 0x60, 0x3c, 0xbd };
};

} // namespace oddf
