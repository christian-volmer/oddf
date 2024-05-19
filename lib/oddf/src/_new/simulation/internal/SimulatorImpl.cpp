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

#include "SimulatorImpl.h"

namespace oddf {
namespace simulation {
namespace internal {

SimulatorImpl::SimulatorImpl()
{
}

SimulatorImpl::~SimulatorImpl()
{
}

bool SimulatorImpl::RegisterSimulatorBlockFactory(design::backend::DesignBlockClass const &designBlockClass,
	std::unique_ptr<backend::SimulatorBlockFactoryBase> &&simulatorBlockFactory)
{
	m_simulatorBlockFactories[designBlockClass] = std::move(simulatorBlockFactory);
	return true;
}

void SimulatorImpl::TranslateDesign(design::backend::IDesign const &design)
{
	auto designBlockEnumerator = design.GetBlockEnumerator();

	while (designBlockEnumerator->MoveNext()) {

		std::cout << designBlockEnumerator->GetCurrent().GetClass().ToString() << " : " << designBlockEnumerator->GetCurrent().GetBlockPath() << "\n";
	}
}

} // namespace internal
} // namespace simulation
} // namespace oddf

#if 0




	/*	temp::BlockEnumerator blocks(design);


	    while (blocks.MoveNext()) {

	        auto const &currentDesignBlock = blocks.GetCurrent();

	        auto designBlockClass = currentDesignBlock.GetClass();

	        auto found = simulatorBlockFactories.find(designBlockClass);

	        if (found != simulatorBlockFactories.end()) {
	        }
	        else {

	            std::cout << "The simulator has no handler for block with class '" << designBlockClass.ToString() << "'.\n";
	        }


	        // ----> need a factories for the different design block classes
	        //m_blockMapping[&currentDesignBlock] = m_blocks.emplace_back(new SimulatorBlockBase(currentDesignBlock)).get();
	    }
	*/

	/*

	MapBlocks
	MapConnections

	ElaborateBlocks -- for all blocks that implement the IElaborate interface (which is passed an IElaborationContext)

	All blocks that implement ... HIER GEHTS WEITER

	BuildComponents
	AllocateNets
	GenerateCode

	*/
}

} // namespace oddf

#endif
