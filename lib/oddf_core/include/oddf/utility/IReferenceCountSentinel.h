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

    Provides interface `IReferenceCountSentinel` for reference counting with
    the purpose of detecting dangling references to deleted objects.

*/

#pragma once

#include <cstddef>

namespace oddf::utility {

/*
    Interface to support reference counting with the purpose of detecting
    dangling references to deleted objects.
*/
class IReferenceCountSentinel {

public:

	// Destructor allows an implementation to throw unless the reference
	// count is zero.
	virtual ~IReferenceCountSentinel() noexcept(false) { }

	// Returns the current value of the reference count.
	virtual size_t GetValue() const noexcept = 0;

	// Increments the reference count.
	virtual void Increment() noexcept = 0;

	// Decrements the reference count. Implementations are required to throw
	// if the reference count becomes negative.
	virtual void Decrement() = 0;
};

} // namespace oddf::utility
