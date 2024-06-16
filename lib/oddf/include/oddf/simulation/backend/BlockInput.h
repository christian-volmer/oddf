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

namespace oddf {
namespace simulation {
namespace backend {

class BlockOutput;
class SimulatorBlockBase;

class BlockInput {

private:

	friend BlockOutput;

	SimulatorBlockBase const *m_owningBlock;
	BlockOutput *m_driver;
	size_t m_index;

public:

	BlockInput(SimulatorBlockBase const *owningBlock, size_t index);

	bool IsConnected() const;
	void ConnectTo(BlockOutput &output);
	void Disconnect();
};

} // namespace backend
} // namespace simulation
} // namespace oddf
