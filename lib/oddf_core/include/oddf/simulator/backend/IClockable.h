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

#include <oddf/IObject.h>

namespace oddf {

namespace simulator::backend {

class IClockable : public virtual IObject {

public:

	// Called by the simulator to simulate a clock cycle.
	virtual void Clock() = 0;
};

} // namespace simulator::backend

template<>
struct Iid<simulator::backend::IClockable> {

	static constexpr Uid value = { 0xb10444ec, 0x414e, 0x4417, 0xaa, 0x37, 0x22, 0xca, 0xc, 0x58, 0xf0, 0x27 };
};

} // namespace oddf
