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

#include "DelayEndpoint.h"

namespace oddf::simulator::common::backend::blocks {

DelayEndpoint::DelayEndpoint(design::blocks::backend::IDesignBlock const *originalDesignBlock, ptrdiff_t busIndex) :
	SimulatorBlockBase(originalDesignBlock, 1, {}),
	m_busIndex(busIndex)
{
}

std::string DelayEndpoint::GetDesignPathHint() const
{
	if (m_busIndex >= 0)
		return GetDesignBlockReference()->GetPath().ToString() + "<" + std::to_string(m_busIndex) + ">:Endpoint";
	else
		return GetDesignBlockReference()->GetPath().ToString() + ":Endpoint";
}

} // namespace oddf::simulator::common::backend::blocks
