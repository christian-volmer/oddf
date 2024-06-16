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

#include <oddf/simulation/backend/SimulatorBlockBase.h>

#include <algorithm>
#include <stdexcept>

namespace oddf {
namespace simulation {
namespace backend {

BlockInput::BlockInput(SimulatorBlockBase const *owningBlock, size_t index) :
	m_owningBlock(owningBlock),
	m_driver(nullptr),
	m_index(index)
{
}

bool BlockInput::IsConnected() const
{
	return m_driver != nullptr;
}

void BlockInput::ConnectTo(BlockOutput &output)
{
	if (IsConnected())
		throw std::logic_error("BlockInput::ConnectTo(): this input is already connected.");

	m_driver = &output;
	output.m_targets.push_back(this);
}

void BlockInput::Disconnect()
{
	if (!IsConnected())
		throw std::logic_error("BlockInput::Disconnect(): this input is unconnected.");

	auto it = std::find(m_driver->m_targets.cbegin(), m_driver->m_targets.cend(), this);

	if (it != m_driver->m_targets.cend())
		m_driver->m_targets.erase(it);
	else
		throw std::runtime_error("BlockInput::Disconnect(): internal error. The input-output connection is inconsistent.");

	m_driver = nullptr;
}

} // namespace backend
} // namespace simulation
} // namespace oddf
