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

    Implements simulator block elaboration whereby existing simulator blocks
    step may create new blocks, for example to better support certain
    combinations of input and output types. A sum taking an integer and a double
    as inputs may convert itself to a block that sums doubles preceded by a
    type-conversion block for the integer input. Another example is where
    operations on busses are individualised.

*/

#include "../SimulatorCore.h"
#include "../SimulatorBlockBase/Internals.h"

#include <oddf/Exception.h>

#include <vector>
#include <memory>
#include <set>
#include <cassert>

namespace oddf::simulator::common::backend {

void SimulatorCore::ElaborateBlocks()
{
	/*
	    Implementation of the `ISimulatorElaborationContext` interface, which
	    becomes passed to the `Elaborate()` member function of the simulator
	    blocks during the elaboration phase.
	*/
	class ElaborationContext : public ISimulatorElaborationContext {

	public:

		// Pointer to the `unique_ptr` to the simulator block under elaboration.
		// Used by member function `RemoveThisBlock()`.
		SimulatorBlockBase *m_currentBlock;

		// Vector of blocks created during elaboration. Becomes moved to the
		// simulator block list.
		std::vector<std::unique_ptr<SimulatorBlockBase>> m_newBlocks;

		// Set of blocks that need to be elaborated again, because their
		// connectivity has changed.
		std::set<SimulatorBlockBase *> m_blocksForReelaboration;

		ElaborationContext() :
			m_currentBlock(nullptr),
			m_newBlocks(),
			m_blocksForReelaboration()
		{
		}

		ElaborationContext(ElaborationContext const &) = delete;
		void operator=(ElaborationContext const &) = delete;

		// Registers `block` to be moved to the simulator block list.
		void AddSimulatorBlock(std::unique_ptr<SimulatorBlockBase> &&block) override
		{
			assert(block);
			m_newBlocks.push_back(std::move(block));
		}

		// Removes the block that is currently under elaboration.
		void RemoveThisBlock() override
		{
			assert(m_currentBlock);

			if (m_currentBlock->m_internals->m_removed)
				throw oddf::Exception(oddf::ExceptionCode::IllegalMethodCall, "ISimulatorElaborationContext::RemoveThisBlock(): the block has already been removed. Was this function accidently called twice?");

			if (m_currentBlock->HasConnections())
				throw oddf::Exception(oddf::ExceptionCode::IllegalMethodCall, "ISimulatorElaborationContext::RemoveThisBlock(): block cannot be removed if it has connections to other blocks.");

			m_blocksForReelaboration.erase(m_currentBlock);
			m_currentBlock->m_internals->m_removed = true;
		}

		virtual void TransferConnectivity(SimulatorBlockInput const &fromInput, SimulatorBlockInput const &toInput) override
		{
			if (&fromInput.GetOwningBlock() != m_currentBlock)
				throw Exception(ExceptionCode::InvalidArgument, "TransferConnectivity(): The input given by parameter 'fromInput' must belong to the block currently under elaboration.");

			if (&fromInput == &toInput)
				return;

			if (fromInput.IsConnected()) {

				auto &fromMutable = fromInput.m_owningBlock.m_internals->m_inputs[fromInput.GetIndex()];
				auto &toMutable = toInput.m_owningBlock.m_internals->m_inputs[toInput.GetIndex()];

				auto &inputDriver = *fromMutable.m_driver;
				fromMutable.Disconnect();
				toMutable.ConnectTo(inputDriver);

				m_blocksForReelaboration.insert(&inputDriver.m_owningBlock);
				m_blocksForReelaboration.insert(&fromInput.m_owningBlock);
				m_blocksForReelaboration.insert(&toInput.m_owningBlock);
			}
		}

		virtual void DisconnectInput(SimulatorBlockInput const &input) override
		{
			if (&input.GetOwningBlock() != m_currentBlock)
				throw Exception(ExceptionCode::InvalidArgument, "DisconnectInput(): The specified input must belong to the block currently under elaboration.");

			if (input.IsConnected()) {

				auto &mutableInput = input.m_owningBlock.m_internals->m_inputs[input.GetIndex()];

				auto &inputDriver = *mutableInput.m_driver;
				mutableInput.Disconnect();

				m_blocksForReelaboration.insert(&inputDriver.m_owningBlock);
				m_blocksForReelaboration.insert(&input.m_owningBlock);
			}
		}

		virtual void TransferConnectivity(SimulatorBlockOutput const &fromOutput, SimulatorBlockOutput const &toOutput) override
		{
			if (&fromOutput.GetOwningBlock() != m_currentBlock)
				throw Exception(ExceptionCode::InvalidArgument, "TransferConnectivity(): The output given by parameter 'fromOuput' must belong to the block currently under elaboration.");

			if (&fromOutput == &toOutput)
				return;

			if (fromOutput.GetType() != toOutput.GetType())
				throw Exception(ExceptionCode::InvalidArgument, "TransferConnectivity(): outputs must have identical types.");

			// Mutable version of `toOutput`
			auto &toOutput_m = toOutput.m_owningBlock.m_internals->m_outputs[toOutput.GetIndex()];

			// The following loop disconnects all inputs (`target`)
			// from `fromOutput` them to `toOutput`

			auto &targets = fromOutput.m_targets;
			while (!targets.empty()) {

				auto *target = targets.front();

				assert(target);

				target->Disconnect();
				target->ConnectTo(toOutput_m);

				m_blocksForReelaboration.insert(&target->m_owningBlock);
			}

			m_blocksForReelaboration.insert(&fromOutput.m_owningBlock);
			m_blocksForReelaboration.insert(&toOutput.m_owningBlock);
		}

		virtual void Connect(SimulatorBlockOutput const &output, SimulatorBlockInput const &toInput)
		{
			if (toInput.IsConnected())
				throw Exception(ExceptionCode::InvalidArgument, "Connect(): input referred to by parameter `toInput` must not be connected.");

			// Mutable versions of the parameters
			auto &output_m = output.m_owningBlock.m_internals->m_outputs[output.GetIndex()];
			auto &toInput_m = toInput.m_owningBlock.m_internals->m_inputs[toInput.GetIndex()];

			toInput_m.ConnectTo(output_m);

			m_blocksForReelaboration.insert(&output.m_owningBlock);
			m_blocksForReelaboration.insert(&toInput.m_owningBlock);
		}
	};

	/*

	The following calls method `Elaborate()` of all simulator blocks.
	The method will also be called on new blocks that become created
	during elaboration. Blocks that become affected by changes in
	connectivity will be elaborated again.

	*/

	size_t current = 0;
	std::set<SimulatorBlockBase *> blocksForReelaboration;

	while (current < m_blocks.size() || !blocksForReelaboration.empty()) {

		auto context = ElaborationContext();

		for (auto *blockForElaboration : blocksForReelaboration) {

			assert(!blockForElaboration->m_internals->m_removed);
			context.m_currentBlock = blockForElaboration;
			blockForElaboration->Elaborate(context);
		}

		blocksForReelaboration.clear();

		while (current < m_blocks.size()) {

			assert(m_blocks[current]);

			if (!m_blocks[current]->m_internals->m_removed) {

				context.m_currentBlock = m_blocks[current].get();
				m_blocks[current]->Elaborate(context);
			}

			if (m_blocks[current]->m_internals->m_removed) {

				/*
				    The block has been removed by a call to `RemoveThisBlock()`.
				    Swap places with the last block of the list, pop the now
				    empty last block from the list, and continue elaboration
				    without incrementing `current`.
				*/

				std::swap(m_blocks[current], m_blocks.back());
				m_blocks.pop_back();
			}
			else
				++current;
		}

		// Append the newly created blocks at the end of the main list...
		// m_blocks.reserve(m_blocks.size() + context.m_newBlocks.size());
		for (auto &newBlock : context.m_newBlocks) {

			// The block will be elaborated anyway in the next elaboration
			// round.

			context.m_blocksForReelaboration.erase(newBlock.get());
			m_blocks.push_back(std::move(newBlock));
		}

		assert(blocksForReelaboration.empty());
		std::swap(blocksForReelaboration, context.m_blocksForReelaboration);
	}

	// Go through all blocks once again and actually remove blocks that have
	// been marked for removal

	current = 0;
	while (current < m_blocks.size()) {

		if (m_blocks[current]->m_internals->m_removed) {

			/*
			    The block has been removed by a call to `RemoveThisBlock()`.
			    Swap places with the last block of the list, pop the now
			    empty last block from the list, and continue elaboration
			    without incrementing `current`.
			*/

			std::swap(m_blocks[current], m_blocks.back());
			m_blocks.pop_back();
		}
		else
			++current;
	}
}

} // namespace oddf::simulator::common::backend
