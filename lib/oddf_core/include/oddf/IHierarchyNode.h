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

    Provides interface `IHierarchyNode` to represent a node in a hierarchy.

*/

#pragma once

#include "IObject.h"
#include "IEnumerator.h"
#include <memory>

namespace oddf {

/*
    Represents a node in a hierarchy tree and provides functions to traverse
    that hierarchy
*/
class IHierarchyNode : public virtual IObject {

public:

	// Returns the name of the node in the hierarchy
	virtual std::string GetName() const = 0;

	// Returns if the node has associated data.
	virtual bool HasData() const noexcept = 0;

	// Returns an `IObject` interface to the associated data. Throws if there is no data.
	virtual std::unique_ptr<IObject> GetData() const = 0;

	// Returns if the node has children
	virtual bool HasChildren() const noexcept = 0;

	// Returns an `IEnumerator` to enumerate the child items.
	virtual std::unique_ptr<IEnumerator<IHierarchyNode const &>> GetChildren() const = 0;

	virtual IHierarchyNode const *GetParent() const = 0;
};

template<>
struct Iid<IHierarchyNode> {

	static constexpr Uid value = { 0x596128ca, 0x7513, 0x453d, 0x85, 0xd2, 0x60, 0xb9, 0x3c, 0x34, 0xe2, 0x7b };
};

} // namespace oddf
