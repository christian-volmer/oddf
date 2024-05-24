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

    Interface for providing an implementation to the ConstIteratorClass class.

*/

#pragma once

namespace oddf {
namespace utility {

template<typename T>
class IConstIteratorImplementation {

public:

	virtual ~IConstIteratorImplementation() { }

	virtual T const &Dereference() const = 0;
	virtual void Increment() = 0;
	virtual bool Equals(IConstIteratorImplementation<T> const &other) const = 0;
};

} // namespace utility
} // namespace oddf