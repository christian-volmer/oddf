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

    Provides template interface `IEnumerator` for enumerating the elements of
    a collection.

*/

#pragma once

namespace oddf::utility {

/*
    Enumeration of the elements of a collection, where the type of the
    underlying container has been erased. Elements are accessed through the
    type specified by template parameter `referenceT`. The container itself
    cannot be modified (i.e., elements cannot be added or deleted); but the
    elements can if `referenceT` specifies a non-const reference or pointer.
*/
template<typename referenceT>
class IEnumerator {

public:

	virtual ~IEnumerator() { }

	// Returns the current element. Throws if there is no current element.
	virtual referenceT GetCurrent() const = 0;

	// Moves the current position of the enumerator to the next valid element.
	// Returns `true` on success and `false` if there is no such element.
	virtual bool MoveNext() = 0;

	// Resets enumeration by placing the current position before the first
	// element. Must call `MoveNext()` to reach a valid position, if available.
	virtual void Reset() = 0;
};

} // namespace oddf::utility
