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

#include "types/Boolean.h"
#include "types/FixedPoint.h"

#include "types/GetRequiredByteSize.h"
#include "types/GetStoredByteSize.h"

namespace oddf::simulator::common::backend::types {

namespace detail {

template<typename T, typename = void>
struct IsValueType_t;

template<typename T>
struct IsValueType_t<T, std::void_t<typename T::ValueType>> : public std::bool_constant<true> {
};

template<typename T>
struct IsValueType_t<T, std::void_t<typename T::ElementType>> : public std::bool_constant<false> {
};

} // namespace detail

template<typename T>
inline bool constexpr IsValueType = detail::IsValueType_t<T>::value;

} // namespace oddf::simulator::common::backend::types
