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

namespace oddf {
namespace simulation {
namespace backend {

SimulatorBlockBase::SimulatorBlockBase(design::backend::IDesignBlock const &designBlock) :
	m_designBlockReference(&designBlock),
	m_inputs(),
	m_outputs()
{
	auto numberOfInputs = designBlock.GetInputsList().size();
	m_inputs.reserve(numberOfInputs);
	for (size_t i = 0; i < numberOfInputs; ++i)
		m_inputs.emplace_back(this, i);

	auto numberOfOutputs = designBlock.GetOutputsList().size();
	m_outputs.reserve(numberOfOutputs);
	for (size_t i = 0; i < numberOfOutputs; ++i)
		m_outputs.emplace_back(this, i);
}

void SimulatorBlockBase::MapConnections(IBlockMapping const &blockMapping)
{
	assert(m_designBlockReference);

	auto designInputIt = m_designBlockReference->GetInputsList().begin();

	for (auto &simInput : m_inputs) {

		auto &designInput = *designInputIt;

		if (designInput.IsConnected()) {

			auto &designDriver = designInput.GetDriver();
			auto &designDrivingBlock = designDriver.GetOwningBlock();
			auto driverIndex = designDriver.GetIndex();

			auto *simDrivingBlock = blockMapping.DesignBlockToSimulatorBlock(designDrivingBlock);
			if (simDrivingBlock) {

				auto &simDriver = simDrivingBlock->m_outputs[driverIndex];
				simInput.ConnectTo(simDriver);
			}
			else {

				std::cout << "Block '" << m_designBlockReference->GetBlockPath() << "': driving block '" << designDrivingBlock.GetBlockPath() << "' not found\n";
			}
		}

		++designInputIt;
	}
}

} // namespace backend
} // namespace simulation
} // namespace oddf
