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

	Implementation of class 'BlockBase', the base class of all blocks in
	the design.

*/

#include "global.h"

#include "node.h"
#include "hierarchy.h"

namespace dfx {

namespace debug {

extern std::string internalLastBlockName;

}

namespace backend {



//
// BlockBase
//

BlockBase::BlockBase(char const *className) :
	name(),
	className(className),
	hierarchyLevel(Design::GetCurrent().GetHierarchy().GetCurrentLevel()),
	mark(false),
	component(nullptr),
	componentNext(nullptr),
	inputPins(),
	outputPins()
{
	name = hierarchyLevel->GenerateBlockName(className);
	dfx::debug::internalLastBlockName = GetName();
}

BlockBase::BlockBase(char const *className, std::string const &tag) :
	name(),
	className(className),
	hierarchyLevel(Design::GetCurrent().GetHierarchy().GetCurrentLevel()),
	mark(false),
	component(nullptr),
	componentNext(nullptr),
	inputPins(),
	outputPins()
{
	name = hierarchyLevel->GenerateBlockName(className);

	if (!tag.empty())
		name += "_" + tag;

	dfx::debug::internalLastBlockName = GetName();
}

BlockBase::~BlockBase()
{
}

std::string BlockBase::GetClassName() const
{
	return className;
}

std::string BlockBase::GetName() const
{
	return name;
}

std::string BlockBase::GetFullName() const
{
	return hierarchyLevel->GetFullName() + "/" + name;
}

std::vector<InputPinBase *> const &BlockBase::GetInputPins() const
{
	return inputPins;
}

std::vector<OutputPinBase *> const &BlockBase::GetOutputPins() const
{
	return outputPins;
}

std::string BlockBase::GetInputPinName(int index) const 
{
	if (index >= 0 && index < (int)GetInputPins().size()) {

		if (GetInputPins().size() == 1)
			return "In";
		else
			return "In" + std::to_string(index);
	}
	
	assert(false);
	return "<ERROR>";
}

std::string BlockBase::GetOutputPinDescription(int index, int &groupIndex, int &busSize, int &busIndex) const
{
	if (index >= 0 && index < (int)GetOutputPins().size()) {

		groupIndex = index;
		busSize = -1;
		busIndex = 0;

		if (GetOutputPins().size() == 1)
			return "Out";
		else
			return "Out" +std::to_string(index);
	}

	assert(false);

	groupIndex = -1;
	busSize = -1;
	busIndex = -1;
	return "<ERROR>";
}

void BlockBase::GetProperties(dfx::generator::Properties &) const
{
}


IStep *BlockBase::GetStep()
{
	return nullptr;
}

void BlockBase::Simplify()
{
}

bool BlockBase::IsTemporary() const
{
	return false;
}

HierarchyLevel const *BlockBase::GetHierarchyLevel() const
{
	return hierarchyLevel;
}

std::string BlockBase::GetHierarchyString() const
{
	return hierarchyLevel->GetFullName();
}

bool BlockBase::CanRemove() const
{
	for (auto const *pin : inputPins)
		if (pin->IsConnected())
			return false;

	for (auto const *pin : outputPins)
		if (pin->IsConnected())
			return false;

	return true;
}

void BlockBase::SetDirty()
{
	if (component)
		component->outdated = true;
}

/*

This is never used

std::size_t BlockBase::GetHash() const
{
	std::size_t hash = 0;

	hash_combine(hash, className);
	hash_combine(hash, inputPins.size());
	hash_combine(hash, outputPins.size());
	
	std::string temp;

	for (auto *output : outputPins)
		hash_combine(hash, output->GetType().GetHash());

	for (auto *input : inputPins) {

		auto driver = input->GetDrivingPin();

		if (driver)
			hash_combine(hash, driver->GetType().GetHash());
		else
			hash_combine(hash, 0);
	}

	return hash;
}

*/

}
}
