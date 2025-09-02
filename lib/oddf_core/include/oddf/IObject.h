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

    Provides interface `IObject`, which provides a generic mechanism for
    obtaining other supported interfaces of the implementing object.

*/

#pragma once

#include "Uid.h"
#include "Iid.h"

namespace oddf {

/*
    Provides a generic mechanism for obtaining other supported interfaces of
    the implementing object.
*/
class IObject {

public:

	virtual ~IObject() = default;

	virtual void *GetInterface(Uid const &iid) = 0;

	void const *GetInterface(Uid const &iid) const
	{
		return const_cast<IObject *>(this)->GetInterface(iid);
	}

	template<typename T>
	T &GetInterface()
	{
		return *static_cast<T *>(GetInterface(Iid<T>::value));
	}

	template<typename T>
	T const &GetInterface() const
	{
		return *static_cast<T const *>(GetInterface(Iid<T>::value));
	}
};

template<>
struct Iid<IObject> {

	static constexpr Uid value = { 0x8a54167e, 0x6e89, 0x4a1a, 0xb2, 0xd5, 0xfc, 0xa9, 0xaa, 0xb, 0xe6, 0xb };
};

} // namespace oddf
