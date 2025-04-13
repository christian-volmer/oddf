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
#include "../SimulatorBlockInternals.h"

#include <oddf/Exception.h>

#include <vector>
#include <memory>
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

		// Pointer to the `unique_ptr` to the simulator block under elaboration. Used by member function `RemoveThisBlock()`.
		std::unique_ptr<SimulatorBlockBase> *m_currentBlockUniquePtr;

		// Vector of blocks created during elaboration. Becomes copied to the simulator block list.
		std::vector<std::unique_ptr<SimulatorBlockBase>> m_newBlocks;

		ElaborationContext() :
			m_currentBlockUniquePtr(nullptr),
			m_newBlocks()
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
			if (!*m_currentBlockUniquePtr)
				throw oddf::Exception(oddf::ExceptionCode::IllegalMethodCall, "ISimulatorElaborationContext::RemoveThisBlock(): the block has already been removed. Was this function accidently called twice?");

			if ((*m_currentBlockUniquePtr)->HasConnections())
				throw oddf::Exception(oddf::ExceptionCode::IllegalMethodCall, "ISimulatorElaborationContext::RemoveThisBlock(): block cannot be removed if it has connections to other blocks.");

			m_currentBlockUniquePtr->reset();
		}

		virtual void TransferConnectivity(SimulatorBlockInput const &fromInput, SimulatorBlockInput const &toInput) override
		{
			if (&fromInput == &toInput)
				return;

			if (fromInput.IsConnected()) {

				auto &fromMutable = fromInput.m_owningBlock.m_internals->m_inputs[fromInput.GetIndex()];
				auto &toMutable = toInput.m_owningBlock.m_internals->m_inputs[toInput.GetIndex()];

				auto &inputDriver = *fromMutable.m_driver;
				fromMutable.Disconnect();
				toMutable.ConnectTo(inputDriver);
			}
		}

		virtual void TransferConnectivity(SimulatorBlockOutput const &fromOutput, SimulatorBlockOutput const &toOutput) override
		{
			if (&fromOutput == &toOutput)
				return;

			if (fromOutput.GetType() != toOutput.GetType())
				throw Exception(ExceptionCode::InvalidArgument, "TransferConnectivity(): outputs must have identical types.");

			auto &toMutable = toOutput.m_owningBlock.m_internals->m_outputs[toOutput.GetIndex()];

			auto &targets = fromOutput.m_targets;

			while (!targets.empty()) {

				auto *target = targets.front();
				target->Disconnect();
				target->ConnectTo(toMutable);
			}
		}
	};

	size_t current = 0;

	do {

		auto context = ElaborationContext();

		while (current < m_blocks.size()) {

			assert(m_blocks[current]);

			context.m_currentBlockUniquePtr = &m_blocks[current];
			m_blocks[current]->Elaborate(context);

			if (m_blocks[current])
				++current;
			else {

				/*
				    The block has been removed by a call to `RemoveThisBlock()`.
				    Swap places with the last block of the list, pop the now
				    empty last block from the list, and continue elaboration
				    without incrementing `current`.
				*/

				std::swap(m_blocks[current], m_blocks.back());
				m_blocks.pop_back();
			}
		}

		// Append the newly created blocks at the end of the main list...
		m_blocks.reserve(m_blocks.size() + context.m_newBlocks.size());
		std::move(std::begin(context.m_newBlocks), std::end(context.m_newBlocks), std::back_inserter(m_blocks));

		// ... and continue elaboration with the newly appended blocks.

	} while (current < m_blocks.size());

	/*

	TODO

	How do we deal with connectivity changes, e.g, blocks with dangling inputs/outputs.
	    - If we disconnect the remaining connections this will cause dangling connections with other blocks...
	    - Maybe connection changes using one of the members of `SimulatorBlockInput` and `SimulatorBlockOutput` should be reported
	      to the simulator core automatically so that affected blocks can be elaborated again?

	*/
}

} // namespace oddf::simulator::common::backend
