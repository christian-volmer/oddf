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

    Defines `SimulatorBlockBase`, which is the base class to all simulator
    blocks.

*/

#pragma once

#include <oddf/design/blocks/backend/IDesignBlock.h>

#include <oddf/simulator/common/backend/ISimulatorElaborationContext.h>
#include <oddf/simulator/common/backend/ISimulatorCodeGenerationContext.h>
#include <oddf/simulator/common/backend/ISimulatorFinalisationContext.h>

#include "SimulatorBlockInput.h"
#include "SimulatorBlockOutput.h"

#include <oddf/utility/IListView.h>

#include <initializer_list>
#include <memory>
#include <string>

namespace oddf::simulator::common::backend {

/*
    Base class to all simulator blocks.
*/
class SimulatorBlockBase {

public:

	class Internals;
	std::unique_ptr<Internals> m_internals;

	// Constructs the simulator block based on the given design block.
	SimulatorBlockBase(design::blocks::backend::IDesignBlock const &designBlock);

	// Constructs the simulator block with given numbers of inputs and outputs.
	SimulatorBlockBase(size_t numberOfInputs, std::initializer_list<design::NodeType> outputNodeTypes);

	virtual ~SimulatorBlockBase();

	// If the block was created based on a design block, will return a pointer to that design block, or nullptr otherwise.
	design::blocks::backend::IDesignBlock const *GetDesignBlockReference() const;

	// Returns a ListView into the list of inputs of this block.
	std::unique_ptr<utility::IListView<SimulatorBlockInput const &>> GetInputsList() const;

	// Returns a ListView into the list of outputs of this block.
	std::unique_ptr<utility::IListView<SimulatorBlockOutput const &>> GetOutputsList() const;

	// Returns whether the block has connections to other block.
	bool HasConnections() const noexcept;

	// Elaborates the block. Implementation should check the number of inputs and outputs and their types.
	virtual void Elaborate(ISimulatorElaborationContext &context) = 0;

	// Generates the final code for this block. Default implementation throws.
	virtual void GenerateCode(ISimulatorCodeGenerationContext &context);

	// Finalises the block. Default implementation does nothing.
	virtual void Finalise(ISimulatorFinalisationContext &context);

	// Returns a descriptive string (similar to a resource path) that hints at the origin of this simulator block in the original design.
	virtual std::string GetDesignPathHint() const = 0;
};

} // namespace oddf::simulator::common::backend
