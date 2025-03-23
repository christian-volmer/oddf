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

#include <oddf/design/NodeType.h>
#include <oddf/utility/CollectionView.h>

#include "Types.h"

#include <list>

namespace oddf::simulator::common::backend {

class SimulatorCore;
class SimulatorBlockInput;
class SimulatorBlockBase;

class SimulatorBlockOutput {

private:

	friend SimulatorCore;
	friend SimulatorBlockInput;
	friend SimulatorBlockBase;

	SimulatorBlockBase &m_owningBlock;
	design::NodeType m_nodeType;
	size_t m_index;

	std::list<SimulatorBlockInput *> m_targets;

	ptrdiff_t m_storageReference;
	void *m_storagePointer;

public:

	~SimulatorBlockOutput() = default;

	SimulatorBlockOutput(SimulatorBlockOutput const &) = delete;
	void operator=(SimulatorBlockOutput const &) = delete;

	SimulatorBlockOutput(SimulatorBlockOutput &&);
	void operator=(SimulatorBlockOutput &&) = delete;

	SimulatorBlockOutput(SimulatorBlockBase &owningBlock, design::NodeType const &nodeType, size_t index);

	design::NodeType GetType() const noexcept;

	template<typename T>
	T const *GetPointer() const
	{
		static_assert(false, "Type `T` must be a simulator type.");
	}

	// Returns the index of this output within the list of outputs of the owning block.
	size_t GetIndex() const noexcept;

	// Returns the simulator block that owns this output.
	SimulatorBlockBase const &GetOwningBlock() const noexcept;

	// Returns a CollectionView into the collection of inputs driven by this block.
	utility::CollectionView<SimulatorBlockInput const &> GetTargetsCollection() const;

	// Returns whether the output drives any inputs.
	bool HasConnections() const noexcept;
};

template<>
types::Boolean const *SimulatorBlockOutput::GetPointer<types::Boolean>() const;

template<>
types::FixedPoint const *SimulatorBlockOutput::GetPointer<types::FixedPoint>() const;

} // namespace oddf::simulator::common::backend
