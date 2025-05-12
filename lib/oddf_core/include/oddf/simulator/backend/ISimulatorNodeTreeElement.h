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
#include <oddf/utility/CollectionView.h>
#include <oddf/IObject.h>

namespace oddf {

namespace simulator::backend {

class ISimulatorNodeTreeElement : public virtual IObject {

public:

	// Returns the name of the current tree element.
	virtual std::string GetName() const = 0;

	// Returns whether the current tree element refers to a simulator node.
	virtual bool IsNode() const = 0;

	virtual design::NodeType GetType() const = 0;

	// Reads from the simulator node.
	virtual void Read(void *buffer, size_t count) const = 0;

	// Returns the collection of child elements. The collection will be empty if there are no children.
	virtual utility::CollectionView<ISimulatorNodeTreeElement const &> GetChildren() const = 0;
};

} // namespace simulator::backend

template<>
struct Iid<simulator::backend::ISimulatorNodeTreeElement> {

	static constexpr Uid value = { 0x14e4b2d5, 0x9884, 0x4dd2, 0xaa, 0x36, 0x42, 0x7e, 0x31, 0x37, 0x75, 0xa7 };
};

} // namespace oddf
