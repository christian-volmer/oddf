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

    Provides function `MakeContainerView()` for obtaining an `IListView<>` or
    `ICollectionView<>` interface for a standard library container.

*/

#pragma once

#include "ElementTransformationNone.h"
#include "IReferenceCountSentinel.h"

#include "backend/StdListView.h"

#include <iterator>
#include <cassert>

namespace oddf::utility {

/*
    Returns a `std::unique_ptr` to an `IListView` or `ICollectionView`
    implementation from iterators `begin_it` and `end_it`. The `size` of the
    collection must be given explicitly. An optional `transformation` to be
    performed on the container elements. An optional `sentinel` interface can be
    provided to guard against dangling references in case the underlying
    container becomes deleted before the collection view returned from this
    function.
*/
template<typename iteratorT, typename transformationT = ElementTransformationNone>
auto inline MakeContainerView(iteratorT begin_it, iteratorT end_it, size_t size, transformationT transformation = {}, IReferenceCountSentinel *sentinel = nullptr)
{
	assert(static_cast<size_t>(std::distance(begin_it, end_it)) == size);

	bool constexpr isRandomAccess = std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<iteratorT>::iterator_category>;

	using implementationT = std::conditional_t<
		isRandomAccess,
		backend::StdListView<transformationT, iteratorT>,
		backend::StdCollectionView<transformationT, iteratorT>>;

	using referenceT = typename implementationT::reference_type;

	using interfaceT = std::conditional_t<
		isRandomAccess,
		IListView<referenceT>,
		ICollectionView<referenceT>>;

	return std::unique_ptr<interfaceT>(new implementationT(begin_it, end_it, size, transformation, sentinel));
}

/*
    Returns a `std::unique_ptr` to an `IListView` or `ICollectionView`
    implementation from the given standard library container. An optional
    `transformation` to be performed on the container elements. An optional
    `sentinel` interface can be provided to guard against dangling references in
    case the underlying container becomes deleted before the collection view
    returned from this function.
*/
template<typename containerT, typename transformationT = ElementTransformationNone>
auto inline MakeContainerView(containerT &container, transformationT transformation = {}, IReferenceCountSentinel *sentinel = nullptr)
{
	// allow both std and ADL-selected overloads
	// see https://en.cppreference.com/w/cpp/iterator/begin.html
	using std::begin, std::end, std::size;

	return MakeContainerView(begin(container), end(container), size(container), transformation, sentinel);
}

} // namespace oddf::utility
