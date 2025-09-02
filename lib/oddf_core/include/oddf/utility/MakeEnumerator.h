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

    Provides function `MakeEnumerator()` for obtaining an `IEnumerator<>`
    interface for a standard library container.

*/

#pragma once

#include "ElementTransformationNone.h"
#include "../IReferenceCountSentinel.h"

#include "backend/StdEnumerator.h"

namespace oddf::utility {

/*
    Returns a `std::unique_ptr` to an `IEnumerator` implementation from
    iterators `begin_it` and `end_it`. An optional `transformation` to be
    performed on the container elements. An optional `sentinel` interface can be
    provided to guard against dangling references in case the underlying
    container becomes deleted before the collection view returned from this
    function.
*/
template<typename iteratorT, typename transformationT = ElementTransformationNone>
auto inline MakeEnumerator(iteratorT begin_it, iteratorT end_it, transformationT proj = {}, IReferenceCountSentinel *sentinel = nullptr)
{
	auto enumerator = new backend::StdEnumerator<transformationT, iteratorT>(begin_it, end_it, proj, sentinel);

	using referenceT = typename std::remove_pointer_t<decltype(enumerator)>::reference_type;
	return std::unique_ptr<IEnumerator<referenceT>>(enumerator);
}

} // namespace oddf::utility
