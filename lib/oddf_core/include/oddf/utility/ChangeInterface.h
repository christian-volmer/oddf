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

    Provides function `ChangeInterface`, which allows to change the interface
    to an object managed by a smart pointer.

*/

#pragma once

#include "../IObject.h"

#include <memory>

namespace oddf::utility {

template<typename toInterfaceT>
inline std::unique_ptr<toInterfaceT> ChangeInterface(std::unique_ptr<IObject> &&from)
{
	auto *pObject = from.release();
	return std::unique_ptr<toInterfaceT>(&pObject->GetInterface<toInterfaceT>());
}

} // namespace oddf::utility
