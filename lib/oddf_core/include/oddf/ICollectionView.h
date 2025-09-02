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

    Provides template interface `ICollectionView`, which is a view into a
    collection of elements.

*/

#pragma once

#include "IEnumerator.h"
#include <memory>

namespace oddf {

/*
    A view into a collection of elements, where the type of the underlying
    container has been erased. Elements are accessed through the type specified
    by template parameter `referenceT`. The container itself cannot be modified
    (i.e., elements cannot be added or deleted); but the elements can if
    `referenceT` specifies a non-const reference or pointer.
*/
template<typename referenceT>
class ICollectionView {

public:

	virtual ~ICollectionView() = default;

	// Returns the number of elements in the collection.
	virtual size_t GetSize() const = 0;

	// Returns whether the collection is empty.
	bool IsEmpty() const
	{
		return GetSize() == 0;
	}

	// Returns the first element of the collection.
	virtual referenceT GetFirst() const = 0;

	// Returns an enumerator for the elements in the collection.
	virtual std::unique_ptr<IEnumerator<referenceT>> GetEnumerator() const = 0;
};

} // namespace oddf
