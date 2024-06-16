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
#include <oddf/simulation/backend/IBlockMapping.h>

#include <cassert>

namespace oddf {
namespace simulation {
namespace internal {

SimulatorImpl::SimulatorImpl()
{
	RegisterDefaultBlockFactories();
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
	class BlockMapping : public backend::IBlockMapping {

		std::unordered_map<design::backend::IDesignBlock const *, backend::SimulatorBlockBase *> m_blockMapping;

	public:

		void AddBlockMapping(design::backend::IDesignBlock const &fromDesignBlock, backend::SimulatorBlockBase &toSimulatorBlock)
		{
			assert(m_blockMapping.find(&fromDesignBlock) == m_blockMapping.end());
			m_blockMapping[&fromDesignBlock] = &toSimulatorBlock;
		}

		virtual backend::SimulatorBlockBase *DesignBlockToSimulatorBlock(design::backend::IDesignBlock const &designBlock) const override
		{
			auto designBlockIt = m_blockMapping.find(&designBlock);

			if (designBlockIt != m_blockMapping.end())
				return designBlockIt->second;
			else
				return nullptr;
		}
	};

	BlockMapping blockMapping;

	auto designBlocks = design.GetBlockCollection();

	//
	// Map blocks
	//

	m_blocks.reserve(designBlocks.size());

	for (auto const &designBlock : designBlocks) {

		auto blockClass = designBlock.GetClass();

		auto blockFactory = m_simulatorBlockFactories.find(blockClass);
		if (blockFactory != m_simulatorBlockFactories.end()) {

			m_blocks.push_back(blockFactory->second->CreateFromDesignBlock(designBlock));
			blockMapping.AddBlockMapping(designBlock, *m_blocks.back());
		}
		else {

			std::cout << "WARNING: do not know how to handle design block '"
					  << designBlock.GetBlockPath()
					  << "' of class '" << blockClass.ToString() << "'.\n";
		}
	}

	//
	// Map connections
	//

	for (auto &pBlock : m_blocks) {

		assert(pBlock);
		auto &block = *pBlock;

		block.MapConnections(blockMapping);
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
