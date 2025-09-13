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

#include "DelayObject.h"

#include <oddf/utility/GetInterfaceHelper.h>

namespace oddf::simulator::common::backend::blocks {

DelayObject::DelayObject(ISimulatorComponent &component) :
	m_component(component),
	m_states()
{
}

DelayElement *DelayObject::AddDelayElement(design::NodeType const &nodeType)
{
	auto state = std::unique_ptr<DelayElement>(new DelayElement(nodeType));
	auto *ptr = state.get();
	m_states.emplace_back(std::move(state));
	return ptr;
}

void *DelayObject::GetInterface(Uid const &iid)
{
	return utility::GetInterfaceHelper<IObject, IClockable>::GetInterface(this, iid);
}

void DelayObject::Clock()
{
	for (auto &state : m_states)
		state->Clock();

	m_component.InvalidateState();
}

} // namespace oddf::simulator::common::backend::blocks
