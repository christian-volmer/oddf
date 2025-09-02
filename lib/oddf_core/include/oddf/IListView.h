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

    Provides template interface `IListView`, a random-access view into a
    collection of elements.

*/

#pragma once

#include "ICollectionView.h"

namespace oddf {

/*
    A random-access view into a collection of elements, where the type of the
    underlying container has been erased. The container itself cannot be
    modified (i.e., elements cannot be added or deleted); but the elements can
    if `referenceT` specifies a non-const reference or pointer. Derives from
    template interface `ICollectionView`.
*/
template<typename referenceT>
class IListView : public ICollectionView<referenceT> {

public:

	// Accesses the element at the given index.
	virtual referenceT Item(size_t index) const = 0;
};

} // namespace oddf
