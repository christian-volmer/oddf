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

#include "ICollectionViewImplementation.h"

#pragma once

namespace oddf {
namespace utility {

template<typename T>
class CollectionView {

protected:

	std::shared_ptr<ICollectionViewImplementation<T>> m_implementation;

public:

	CollectionView(std::shared_ptr<ICollectionViewImplementation<T>> const &implementation) :
		m_implementation(implementation)
	{
	}

	Iterator<T> begin() const { return m_implementation->begin(); }
	Iterator<T> end() const { return m_implementation->end(); }

	size_t size() const { return m_implementation->size(); }
};

} // namespace utility
} // namespace oddf
