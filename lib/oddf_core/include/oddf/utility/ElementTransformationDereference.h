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

    Provides class `ElementTransformationDereference` that dereferences
    pointer and pointer-like elements with our standard view implementations of
    standard library containers.

*/

#pragma once

namespace oddf::utility {

/*
    Dereferences pointer and pointer-like elements with our standard view
    implementations of standard library containers.
*/
struct ElementTransformationDereference {

	template<typename T>
	auto &operator()(T &t) const
	{
		return *t;
	}

	template<typename T>
	auto const &operator()(T const &t) const
	{
		return *t;
	}
};

} // namespace oddf::utility
