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

#include "DelayElement.h"

#include <oddf/simulator/common/backend/types/Support.h>

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

DelayElement::DelayElement(design::NodeType const &nodeType) :
	m_nodeType(nodeType),
	m_source(),
	m_dataSize(types::GetDataSizeForNodeType(nodeType)),
	m_state()
{
	m_state.reset(new unsigned char[m_dataSize] {});
}

void DelayElement::Clock()
{
	memcpy(m_state.get(), m_source, m_dataSize);

	// This is an internal data consistency check, which should never fire.
	if (!types::CheckDataIntegrityForNodeType(m_state.get(), m_dataSize, m_nodeType))
		throw Exception(ExceptionCode::Unexpected);
}

} // namespace oddf::simulator::common::backend::blocks
