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

    Declaration of class 'BlockBase', the base class of all blocks in the
    design.

*/

#pragma once

#include <oddf/design/backend/IDesignBlock.h>

namespace dfx {

struct HierarchyLevel;
class Simulator;
template<typename T>
class node;

namespace generator {

class Properties;

} // namespace generator

namespace backend {

class Component;
class InputPinBase;
class OutputPinBase;

//
// IStep: interface for clocked blocks.
//

class IStep {

public:

	virtual void Step() = 0;
	virtual void AsyncReset() = 0;
};

//
// BlockBase
//

class BlockBase : public oddf::design::backend::IDesignBlock {

public:

	using source_blocks_t = std::unordered_set<BlockBase *>;

private:

	std::string name;
	std::string className;
	HierarchyLevel *hierarchyLevel;

private:

	bool mark;
	Component *component;
	BlockBase *componentNext;

	friend class dfx::Simulator;

private:

	std::vector<InputPinBase *> inputPins;

	friend class InputPinBase;

private:

	std::vector<OutputPinBase *> outputPins;

	friend class OutputPinBase;

protected:

	virtual std::string GetInputPinName(int index) const;
	virtual std::string GetOutputPinDescription(int index, int &groupIndex, int &busSize, int &busIndex) const;

public:

	BlockBase(char const *className);
	BlockBase(char const *className, std::string const &tag);
	BlockBase(BlockBase const &) = delete;
	virtual ~BlockBase();

	BlockBase &operator=(BlockBase const &) = delete;

	std::string GetName() const;
	std::string GetClassName() const;
	std::string GetFullName() const;

	std::vector<InputPinBase *> const &GetInputPins() const;
	std::vector<OutputPinBase *> const &GetOutputPins() const;

	virtual void GetProperties(dfx::generator::Properties &properties) const;

	HierarchyLevel const *GetHierarchyLevel() const;
	std::string GetHierarchyString() const;

	// Evaluates this block and updates the values of its output nodes.
	virtual void Evaluate() = 0;

	// Returns all input blocks whose results are required for the evaluation of the current block. Used by the simulator to determine the evaluation order.
	virtual source_blocks_t GetSourceBlocks() const = 0;

	// Returns an IStep interface if the block is clocked or nullptr otherwise.
	virtual IStep *GetStep();

	// Indicates whether Evaluate() should be called during simulation.
	virtual bool CanEvaluate() const = 0;

	// Called once by the simulator to remove 'identity' blocks
	virtual void Simplify();

	// Indicates a temporary block
	virtual bool IsTemporary() const;

	// Indicates whether a block has no function and can therefore be safely removed from the design.
	// Default implementation returns 'true' if the block does not have any connections to other blocks.
	virtual bool CanRemove() const;

	void SetDirty();

	/*

	This is never used.

	// Returns a hash based on the the class name, the number of inputs and outputs and their data types, and the properties.
	// Can be overidden in a derived class to include additional parameters in the computation.
	virtual std::size_t GetHash() const;

	*/

	//
	// IDesignBlock interface implementation
	//

	virtual std::string GetBlockPath() const override
	{
		return GetFullName();
	}

	virtual oddf::design::backend::DesignBlockClass GetClass() const override
	{
		return GetClassName();
	}

	virtual oddf::utility::ListView<oddf::design::backend::IBlockInput> GetInputsList() const override;
	virtual oddf::utility::ListView<oddf::design::backend::IBlockOutput> GetOutputsList() const override;
};

} // namespace backend
} // namespace dfx
