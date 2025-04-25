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

#include <oddf/Uid.h>
#include <oddf/IObject.h>

#include <string>

namespace oddf {

namespace design::blocks::backend {

class ITaggedBlock : public virtual IObject {

public:

	// Returns the tag given to the block.
	virtual std::string GetTag() const = 0;
};

} // namespace design::blocks::backend

template<>
struct Iid<design::blocks::backend::ITaggedBlock> {

	static constexpr Uid value = { 0x6245db36, 0x98ff, 0x403a, 0xb1, 0x3, 0x4b, 0x9d, 0xeb, 0xe8, 0x6, 0xf7 };
};

} // namespace oddf
