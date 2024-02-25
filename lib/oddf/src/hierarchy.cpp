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

	Implementation of the design hierarchy, which supports named
	instances of modules.

*/

#include "global.h"

#include "node.h"
#include "hierarchy.h"
#include "messages.h"

#include "formatting.h"


namespace dfx {

namespace debug {

extern std::string internalCurrentHierarchyName;

}


//
// hierarchy_level
//

HierarchyLevel::HierarchyLevel() :
	sequenceNumber(0),
	name(),
	parent(nullptr),
	next(nullptr),
	firstChild(nullptr)
{
}

std::string HierarchyLevel::GenerateBlockName(std::string const &prefix)
{
	int counter = ++blockNameCounters[prefix];
	return string_printf(prefix + "%d", counter);
}


int HierarchyLevel::GetSequenceNumber() const
{
	return sequenceNumber;
}

std::string const &HierarchyLevel::GetName() const
{
	return name;
}

std::string HierarchyLevel::GetFullName() const
{
	HierarchyLevel const *current = parent;
	std::string fullName = name;

	while (current != nullptr) {

		fullName = current->name + "/" + fullName;
		current = current->parent;
	}

	return fullName;
}

std::string const &HierarchyLevel::GetModuleName() const
{
	return moduleName;
}

HierarchyLevel *HierarchyLevel::GetParent() const
{
	return parent;
}




//
// hierarchy
//

Hierarchy::Hierarchy(Design *theDesign) :
	design(theDesign),
	root(),
	current(&root)
{
	root.name = "Root";
	debug::internalCurrentHierarchyName = GetCurrentLevel()->GetFullName();
}

Hierarchy::~Hierarchy()
{
	RecursiveDelete(root.firstChild);
}

void Hierarchy::RecursiveDelete(HierarchyLevel *level)
{
	while (level != nullptr) {

		HierarchyLevel *next = level->next;

		RecursiveDelete(level->firstChild);

		delete level;
		level = next;
	}
}

void Hierarchy::RecursivePrintTree(std::basic_ostream<char> &os, HierarchyLevel const *level, int indentation) const
{
	using std::endl;
	using std::setw;

	while (level != nullptr) {

		os << setw(3*indentation) << "" << level->name << endl;
		RecursivePrintTree(os, level->firstChild, indentation + 1);

		level = level->next;
	}
}

void Hierarchy::Report(std::basic_ostream<char> &os) const
{
	using std::endl;

	os << endl << " --- Hierarchy --- " << endl << endl;

	RecursivePrintTree(os, &root, 1);

	os << endl;
}

HierarchyLevel *Hierarchy::GetCurrentLevel() const
{
	return current;
}

void Hierarchy::BeginNewLevel(std::string const &name, std::string const &moduleName)
{
	// Check if instance name already exists
	int sequenceNumber = 0;
	HierarchyLevel **ppLevel = &current->firstChild;
	while (*ppLevel != nullptr) {
		if ((*ppLevel)->name == name)
			throw design_error("Instance '" + (*ppLevel)->GetFullName() + "' already exists");
		ppLevel = &(*ppLevel)->next;	
		++sequenceNumber;
	}

	HierarchyLevel *newInstance = new HierarchyLevel();
	newInstance->sequenceNumber = sequenceNumber;
	newInstance->name = name;
	newInstance->moduleName = moduleName;
	newInstance->firstChild = nullptr;
	newInstance->parent = current;
	newInstance->next = nullptr;

	// Insert new instance at the end of the list
	*ppLevel = newInstance;

	current = newInstance;
}


//
// ScopedHierarchyLevel
//

ScopedHierarchyLevel::ScopedHierarchyLevel(std::string const &name, std::string const &moduleName) :
hierarchy(Design::GetCurrent().GetHierarchy()),
previousLevel(nullptr)
{
	previousLevel = hierarchy.current;
	hierarchy.BeginNewLevel(name, moduleName);
	debug::internalCurrentHierarchyName = hierarchy.GetCurrentLevel()->GetFullName();
}

ScopedHierarchyLevel::~ScopedHierarchyLevel()
{
	hierarchy.current = previousLevel;
	debug::internalCurrentHierarchyName = hierarchy.GetCurrentLevel()->GetFullName();
}


//
// ScopedGotoHierarchyLevel
//

ScopedGotoHierarchyLevel::ScopedGotoHierarchyLevel(HierarchyLevel *gotoLevel) :
	hierarchy(Design::GetCurrent().GetHierarchy()),
	previousLevel(nullptr)
{
	assert(gotoLevel != nullptr);
	previousLevel = hierarchy.current;
	hierarchy.current = gotoLevel;
	debug::internalCurrentHierarchyName = hierarchy.GetCurrentLevel()->GetFullName();
}

ScopedGotoHierarchyLevel::~ScopedGotoHierarchyLevel()
{
	hierarchy.current = previousLevel;
	debug::internalCurrentHierarchyName = hierarchy.GetCurrentLevel()->GetFullName();
}


}
