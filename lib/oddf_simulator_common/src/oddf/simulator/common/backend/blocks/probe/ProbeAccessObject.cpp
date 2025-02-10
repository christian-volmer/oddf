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

#include "ProbeAccessObject.h"

#include <oddf/simulator/common/backend/Types.h>

#include <oddf/utility/CopyBoolean.h>
#include <oddf/utility/CopyInteger.h>

namespace oddf::simulator::common::backend::blocks {

//
// types::Boolean
//

template<>
void ProbeAccessObject<types::Boolean>::Read(void *buffer, size_t count) const
{
	m_component.EnsureValidState();
	utility::CopyBoolean(buffer, count, m_probedOutputPointer, types::GetStoredByteSize(m_nodeType));
}

//
// types::FixedPointElement
//

template<>
void ProbeAccessObject<types::FixedPointElement>::Read(void *buffer, size_t count) const
{
	m_component.EnsureValidState();
	if (m_nodeType.IsSigned())
		utility::CopySignedInteger(buffer, count, m_probedOutputPointer, types::GetStoredByteSize(m_nodeType));
	else
		utility::CopyUnsignedInteger(buffer, count, m_probedOutputPointer, types::GetStoredByteSize(m_nodeType));
}

} // namespace oddf::simulator::common::backend::blocks
