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

#include "IIteratorImplementation.h"

namespace oddf {
namespace utility {

template<typename T>
class Iterator {

	std::unique_ptr<IIteratorImplementation<T>> m_implementation;

public:

	Iterator(std::unique_ptr<IIteratorImplementation<T>> &&implementation) :
		m_implementation(std::move(implementation)) { }

	Iterator(Iterator<T> const &other) :
		m_implementation(other.m_implementation->Clone()) { }

	T const &operator*() const { return m_implementation->Dereference(); }

	Iterator<T> &operator++()
	{
		m_implementation->Increment();
		return *this;
	}

	void operator++(int) { m_implementation->Increment(); }

	bool operator==(Iterator<T> const &other) const { return m_implementation->Equals(*other.m_implementation); }
	bool operator!=(Iterator<T> const &other) const { return !m_implementation->Equals(*other.m_implementation); }
};

} // namespace utility
} // namespace oddf
