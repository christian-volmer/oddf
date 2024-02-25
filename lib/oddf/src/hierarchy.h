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

	Classes for the design hierarchy, which supports named instances of
	modules.

*/

#pragma once

namespace dfx {

class Design;

//
// HierarchyLevel class
//

struct HierarchyLevel {

private:

	int sequenceNumber;
	std::string name;
	std::string moduleName;

	HierarchyLevel *parent;
	HierarchyLevel *next;
	HierarchyLevel *firstChild;

	friend class Hierarchy;

private:

	std::unordered_map<std::string, int> blockNameCounters;

public:

	HierarchyLevel();
	
	int GetSequenceNumber() const;
	std::string const &GetName() const;
	std::string GetFullName() const;

	std::string const &GetModuleName() const;

	HierarchyLevel *GetParent() const;

	std::string GenerateBlockName(std::string const &prefix);

	HierarchyLevel(HierarchyLevel const &) = delete;
	HierarchyLevel(HierarchyLevel &&) = delete;
	void operator =(HierarchyLevel const &) = delete;
	void operator =(HierarchyLevel &&) = delete;
};


//
// hierarchy class
//

class Hierarchy {

private:

	Design *design;

	HierarchyLevel root;
	HierarchyLevel *current;

	void BeginNewLevel(std::string const &name, std::string const &moduleName);

	void RecursiveDelete(HierarchyLevel *level);
	void RecursivePrintTree(std::basic_ostream<char> &os, HierarchyLevel const *level, int indentation) const;

	friend class ScopedHierarchyLevel;
	friend class ScopedGotoHierarchyLevel;

public:

	Hierarchy(Design *theDesign);
	~Hierarchy();

	HierarchyLevel *GetCurrentLevel() const;

	void Report(std::basic_ostream<char> &os) const;

	Hierarchy(Hierarchy const &) = delete;
	Hierarchy(Hierarchy &&) = delete;
	void operator =(Hierarchy const &) = delete;
	void operator =(Hierarchy &&) = delete;
};


//
// ScopedHierarchyLevel class
//

class ScopedHierarchyLevel {

private:

	Hierarchy &hierarchy;
	HierarchyLevel *previousLevel;

public:

	ScopedHierarchyLevel(std::string const &name, std::string const &moduleName);
	ScopedHierarchyLevel(ScopedHierarchyLevel const &) = delete;
	~ScopedHierarchyLevel();

	void operator=(ScopedHierarchyLevel const &) = delete;
};

#define DFX_INSTANCE(instanceName, moduleName) \
	dfx::ScopedHierarchyLevel _instance(instanceName, moduleName); \
	if (dfx::Design::GetCurrent().hasCustomDefaultEnable) \
		dfx::blocks::_Label(dfx::Design::GetCurrent().customDefaultEnable, "input", "ClockEnable")

//
// ScopedGotoHierarchyLevel class
//

class ScopedGotoHierarchyLevel {

private:

	Hierarchy &hierarchy;
	HierarchyLevel *previousLevel;

public:

	ScopedGotoHierarchyLevel(HierarchyLevel *gotoLevel);
	ScopedGotoHierarchyLevel(ScopedHierarchyLevel const &) = delete;
	~ScopedGotoHierarchyLevel();

	void operator=(ScopedGotoHierarchyLevel const &) = delete;
};


}
