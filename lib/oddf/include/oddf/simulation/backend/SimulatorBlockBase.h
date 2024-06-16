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

#include <oddf/design/backend/IDesignBlock.h>

#include "BlockInput.h"
#include "BlockOutput.h"
#include "IBlockMapping.h"

#include <vector>

namespace oddf {
namespace simulation {
namespace backend {

class SimulatorBlockBase {

private:

	design::backend::IDesignBlock const *m_designBlockReference;

	std::vector<BlockInput> m_inputs;
	std::vector<BlockOutput> m_outputs;

public:

	SimulatorBlockBase(design::backend::IDesignBlock const &designBlock);

	void MapConnections(IBlockMapping const &blockMapping);

	virtual ~SimulatorBlockBase() { }
};

} // namespace backend
} // namespace simulation
} // namespace oddf
