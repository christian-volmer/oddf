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

#include "../SimulatorCore.h"

#include <oddf/Exception.h>

#include <unordered_map>
#include <cassert>

namespace oddf::simulator::common::backend {

namespace {

class BlockMapping : public ISimulatorBlockMapping {

	std::unordered_map<design::blocks::backend::IDesignBlock const *, SimulatorBlockBase *> m_blockMapping;

public:

	BlockMapping() :
		m_blockMapping()
	{
	}

	void AddBlockMapping(design::blocks::backend::IDesignBlock const &fromDesignBlock, SimulatorBlockBase &toSimulatorBlock)
	{
		assert(m_blockMapping.find(&fromDesignBlock) == m_blockMapping.end());
		m_blockMapping[&fromDesignBlock] = &toSimulatorBlock;
	}

	virtual SimulatorBlockBase *DesignBlockToSimulatorBlock(design::blocks::backend::IDesignBlock const &designBlock) const override
	{
		auto designBlockIt = m_blockMapping.find(&designBlock);

		if (designBlockIt != m_blockMapping.end())
			return designBlockIt->second;
		else
			return nullptr;
	}
};

} // namespace

std::unique_ptr<ISimulatorBlockMapping const> SimulatorCore::MapBlocks(design::IDesign const &design)
{
	auto pBlockMapping = std::make_unique<BlockMapping>();

	auto designBlocks = design.GetBlockCollection();

	m_blocks.reserve(designBlocks->GetSize());

	for (auto designBlocksEnumerator = designBlocks->GetEnumerator(); designBlocksEnumerator->MoveNext();) {

		auto const &designBlock = designBlocksEnumerator->GetCurrent();

		auto blockClass = designBlock.GetClass();

		auto blockFactory = m_simulatorBlockFactories.find(blockClass);
		if (blockFactory != m_simulatorBlockFactories.end()) {

			m_blocks.push_back(blockFactory->second->CreateFromDesignBlock(designBlock));
			pBlockMapping->AddBlockMapping(designBlock, *m_blocks.back());
		}
		else {

			throw Exception(ExceptionCode::Unsupported,
				"Do not know how to handle design block '"
					+ designBlock.GetPath().ToString()
					+ "' of class '"
					+ blockClass.ToString()
					+ "'.");
		}
	}

	return pBlockMapping;
}

} // namespace oddf::simulator::common::backend
