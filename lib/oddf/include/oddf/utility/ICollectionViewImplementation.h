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

#include "Iterator.h"

#pragma once

namespace oddf {
namespace utility {

template<typename T>
class ICollectionViewImplementation {

public:

	virtual ~ICollectionViewImplementation() { }

	virtual Iterator<T> begin() const = 0;
	virtual Iterator<T> end() const = 0;

	virtual size_t size() const = 0;
};

} // namespace utility
} // namespace oddf
