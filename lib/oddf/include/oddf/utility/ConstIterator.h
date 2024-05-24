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

    Generic implementation of an iterator that is compatible with the standard library and that
    does not depend on the data type of the container.

*/

#pragma once

#include "IConstIteratorImplementation.h"

#include <iterator>
#include <memory>

namespace oddf {
namespace utility {

template<typename T>
class ConstIterator {

private:

	std::shared_ptr<IConstIteratorImplementation<T>> m_implementation;

public:

	using iterator_category = std::forward_iterator_tag;
	using value_type = T const;
	using difference_type = ptrdiff_t;
	using pointer = T const *;
	using reference = T const &;

	ConstIterator(std::shared_ptr<IConstIteratorImplementation<T>> implementation) :
		m_implementation(implementation)
	{
	}

	T const &operator*() const
	{
		return m_implementation->Dereference();
	}

	ConstIterator<T> const &operator++()
	{
		m_implementation->Increment();
		return *this;
	}

	void operator++(int)
	{
		m_implementation->Increment();
	}

	bool operator==(ConstIterator<T> const &other) const
	{
		return m_implementation->Equals(*other.m_implementation);
	}

	bool operator!=(ConstIterator<T> const &other) const
	{
		return !m_implementation->Equals(*other.m_implementation);
	}
};

} // namespace utility
} // namespace oddf
