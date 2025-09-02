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

#include <oddf/design/NodeType.h>

namespace oddf {

namespace simulator::backend {

class IProbeAccess : public virtual IObject {

public:

	virtual design::NodeType GetType() const noexcept = 0;

	virtual void Read(void *buffer, size_t count) const = 0;
	virtual size_t GetSize() const noexcept = 0;
};

} // namespace simulator::backend

template<>
struct Iid<simulator::backend::IProbeAccess> {

	static constexpr Uid value = { 0x695f4116, 0x335e, 0x47ed, 0xa2, 0xf, 0xc8, 0x29, 0x94, 0x59, 0xe6, 0x8e };
};

} // namespace oddf
