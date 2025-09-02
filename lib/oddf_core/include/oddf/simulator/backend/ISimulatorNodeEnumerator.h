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

    Provides `ISimulatorNodeEnumerator` for enumerating `ISimulatorNode`
    interfaces to nodes in the simulator.

*/

#pragma once

#include "ISimulatorNode.h"

#include <oddf/IEnumerator.h>

namespace oddf {

namespace simulator::backend {

/*
    Enumerates `ISimulatorNode` interfaces to nodes in the simulator. Specialises
    `IEnumerator` and derives from `IObject`.
 */
class ISimulatorNodeEnumerator : public virtual IObject, public virtual IEnumerator<ISimulatorNode const &> {
};

} // namespace simulator::backend

template<>
struct Iid<simulator::backend::ISimulatorNodeEnumerator> {

	static constexpr Uid value = { 0x14de35ff, 0x447d, 0x4bfb, 0x80, 0xb1, 0x42, 0x4b, 0x3f, 0xd4, 0x59, 0x33 };
};

} // namespace oddf
