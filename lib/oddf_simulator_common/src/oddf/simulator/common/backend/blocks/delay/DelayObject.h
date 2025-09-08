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

#include "DelayElement.h"

#include <oddf/simulator/common/backend/ISimulatorComponent.h>

#include <oddf/simulator/backend/IClockable.h>

#include <oddf/design/NodeType.h>

#include <oddf/Clsid.h>

#include <list>
#include <memory>

namespace oddf {

namespace simulator::common::backend::blocks {

class DelayObject : public virtual simulator::backend::IClockable {

private:

	ISimulatorComponent &m_component;

	std::list<std::unique_ptr<DelayElement>> m_states;

public:

	DelayObject(ISimulatorComponent &component);

	DelayElement *AddDelayElement(design::NodeType const &nodeType);

	//
	// IObject member
	//

	virtual void *GetInterface(Uid const &iid) override;

	//
	// IClockable member
	//

	virtual void Clock() override;
};

} // namespace simulator::common::backend::blocks

template<>
struct Clsid<simulator::common::backend::blocks::DelayObject> {

	static constexpr Uid value = { 0x17fb33b1, 0x928f, 0x44af, 0xb9, 0x30, 0xff, 0xec, 0xb8, 0x44, 0x1f, 0xb2 };
};

} // namespace oddf
