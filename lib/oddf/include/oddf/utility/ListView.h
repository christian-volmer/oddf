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

#include "CollectionView.h"
#include "IListViewImplementation.h"

#pragma once

namespace oddf {
namespace utility {

template<typename T>
class ListView : public CollectionView<T> {

	using CollectionView<T>::m_implementation;

public:

	ListView(std::shared_ptr<IListViewImplementation<T>> const &implementation) :
		CollectionView<T>(implementation)
	{
	}

	T const &operator[](size_t pos) const
	{
		return static_cast<IListViewImplementation<T> *>(m_implementation.get())->at(pos);
	}
};

} // namespace utility
} // namespace oddf
