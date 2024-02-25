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

	Classes and interfaces that support the creation of read and write
	registers and mapped memory access with automatic address generation
	and support for hierarchical blocks.

*/

#include "../global.h"

#include "configuration.h"

namespace dfx {
namespace configuration {

void Namespace::AddEntry(Entry::Access access, std::string const &name, std::string const &description, HierarchyLevel const *instance, types::TypeDescription const &typeDesc, int address, int arrayLength)
{
	std::string fullName = instance->GetFullName() + "/" + name;

	if (nameLookup.find(fullName) == nameLookup.end()) {

		Entry newEntry;

		newEntry.access = access;
		newEntry.name = name;
		newEntry.description = description;
		newEntry.typeDesc = typeDesc;
		newEntry.instance = instance;
		newEntry.address = address;
		newEntry.arrayLength = arrayLength;
		newEntry.fullName = fullName;

		nameLookup[fullName] = (int)entries.size();
		entries.push_back(std::move(newEntry));
	}
	else
		throw design_error("An entry with name '" + fullName + "' already exists.");
}

Namespace::Entry const *Namespace::Lookup(std::string const &fullName) const
{
	auto it = nameLookup.find(fullName);
	return it != nameLookup.end() ? &entries[it->second] : nullptr;
}

void Namespace::Report(std::basic_ostream<char> &os) const
{
	using std::setw;
	using std::endl;

	os << endl << " --- Configuration Namespace --- " << endl;

	HierarchyLevel const *previousInstance = nullptr;

	for (auto const &entry : entries) {

		HierarchyLevel const *newInstance = entry.instance;

		if (newInstance != previousInstance) {

			os << endl << " " << newInstance->GetFullName() << endl << endl;
			previousInstance = newInstance;
		}

		char const *description = entry.description.empty() ? "<no description>" : entry.description.c_str();

		char const *access;
		switch (entry.access) {

			case Entry::Access::READ_WRITE:
				access = "ReadWrite";
				break;

			case Entry::Access::READ_ONLY:
				access = "ReadOnly";
				break;

			case Entry::Access::WRITE_ONLY:
				access = "WriteOnly";
				break;

			default:
				access = "<ERROR>";
				assert(false);
				break;
		}

		std::string bounds;
		if (entry.arrayLength > 1)
			bounds = "[" + std::to_string(entry.arrayLength) + "]";

		os << string_printf("%7d %30s %15s %20s %-8s %s\n", entry.address, entry.name.c_str(), access, entry.typeDesc.ToString().c_str(), bounds.c_str(), description);
	}

	os << endl;
}

void Namespace::Table(std::basic_ostream<char> &os, std::string const &nSpaceArg) const
{
	using std::setw;
	using std::endl;

	os << "#pragma once" << endl;
	
	os << endl << "#include \"shared/RegisterTypes.h\"" << endl;
	os << "#ifndef REGISTER_RANDOMCHECKS" << endl;
	os << " #define REGISTER_RANDOMCHECKS 4" << endl;
	os << "#endif" << endl;
	os << "#ifndef ROM_RANDOMCHECKS" << endl;
	os << " #define ROM_RANDOMCHECKS 10" << endl;
	os << "#endif" << endl;
	os << "#ifndef RAM_RANDOMCHECKS" << endl;
	os << " #define RAM_RANDOMCHECKS 10" << endl;
	os << "#endif" << endl;
	os << endl;

	os << "class " << nSpaceArg << " { " << endl;
	os << endl;
	os << "public: " << endl;
	os << " static int BaseAddress;" << endl;


	os << endl << "std::vector<Register> RegisterTable = { " << endl;
	os << string_printf("//%6s|%10s|%11s|%11s|%c%s%c\n","Addr","WordWidth","ArrayLength","AccessType",'\"',"RegisterName", '\"');

	HierarchyLevel const *previousInstance = nullptr;
	int n = 0;

	for (auto const &entry : entries ) {

		HierarchyLevel const *newInstance = entry.instance;

		if (newInstance != previousInstance) {

			//os << endl << " " << newInstance->GetFullName() << endl << endl;
			previousInstance = newInstance;
		}


		char const *access;
		switch (entry.access) {

		case Entry::Access::READ_WRITE:
			access = "ReadWrite";
			break;

		case Entry::Access::READ_ONLY:
			access = "ReadOnly";
			break;

		case Entry::Access::WRITE_ONLY:
			access = "WriteOnly";
			break;

		default:
			access = "<ERROR>";
			assert(false);
			break;
		}

		//std::string bounds;
		//if (entry.arrayLength > 1)
		//	bounds = "[" + std::to_string(entry.arrayLength) + "]";
		
		int wordWidth;
		if (entry.typeDesc.IsClass(dfx::types::TypeDescription::Boolean))   wordWidth = 1;
		else                                                                wordWidth = entry.typeDesc.GetWordWidth();

		if (++n != (int)entries.size())
		{ 
          os << string_printf("{%7d,%10d,%11d,%11s,%c%s%c},\n", entry.address, wordWidth, entry.arrayLength, access,'\"', entry.fullName.c_str(),'\"');

		}
		else
		{
          os << string_printf("{%7d,%10d,%11d,%11s,%c%s%c}};\n", entry.address, wordWidth, entry.arrayLength, access,'\"', entry.fullName.c_str(),'\"');
		 //os << "Finished after " << n << " vector entries !" << std::endl;
		}
	}
    
	os << endl;
	os << "}; //" << nSpaceArg << endl;

}


}
}
