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

#pragma once

#include "../global.h"
#include "../types.h"

namespace dfx {
namespace configuration {

class Namespace;

// TODO: HACK: these should be template parameters, actually...
int const AddressWidth = 19;
int const DataWidth = 32;
using AddressT = ufix<AddressWidth>;
using DataT = sfix<DataWidth>;


//
// Range interface
//

class Range {

public:

	inout::output_node<bool> ClearAll;
	inout::output_node<dynfix> Address;
	inout::output_node<bool> WriteEnable;
	inout::output_node<dynfix> WriteData;
	inout::output_node<bool> ReadRequest;

	inout::input_node<bool> ReadAcknowledge;
	inout::input_node<dynfix> ReadData;

	Range() = delete;
	Range(bool isSigned, int wordWidth, int fraction) :
		ClearAll(),
		Address(),
		WriteEnable(),
		WriteData(),
		ReadRequest(),
		ReadAcknowledge(),
		ReadData(dynfix(isSigned, wordWidth, fraction))
	{
	}

	Range(Range &&other) :
		ClearAll(std::move(other.ClearAll)),
		Address(std::move(other.Address)),
		WriteEnable(std::move(other.WriteEnable)),
		WriteData(std::move(other.WriteData)),
		ReadRequest(std::move(other.ReadRequest)),
		ReadAcknowledge(std::move(other.ReadAcknowledge)),
		ReadData(std::move(other.ReadData))
	{
	}
};

//
// IBuilder interface
//

class IBuilder {

public:

	virtual void Break(int extraPipelining) = 0;
	virtual void Merge(int extraPipelining) = 0;

	virtual Namespace &GetNamespace() = 0;
	virtual int GetCurrentAddress() = 0;

	virtual node<dynfix> AddRegister(bool isSigned, int wordWidth, int fraction, std::string const &tag) = 0;
	virtual void AddObserver(node<dynfix> const &source, std::string const &tag) = 0;
	virtual Range AddRange(bool isSigned, int wordWidth, int fraction, int length, std::string const &tag) = 0;
};


//
// IController interface
//

class IController {

public:

	virtual void ClearAll() = 0;
	virtual void Write(int theAddress, std::uint32_t const *values, int count) = 0;
	virtual void Read(int theAddress, std::uint32_t *values, int count) = 0;
};


//
// Namespace
//

class Namespace {

public:

	struct Entry {

		enum Access {

			READ_ONLY = 1,
			WRITE_ONLY = 2,
			READ_WRITE = 3,
		};

		Access access;
		std::string name;
		std::string description;
		types::TypeDescription typeDesc;
		HierarchyLevel const *instance;
		int address;
		int arrayLength;
		std::string fullName;
	};

public:

	std::unordered_map<std::string, int> nameLookup;
	std::vector<Entry> entries;

public:

	void AddEntry(Entry::Access access, std::string const &name, std::string const &description, HierarchyLevel const *instance, types::TypeDescription const &typeDesc, int address, int arrayLength);
	Entry const *Lookup(std::string const &fullName) const;

	void Report(std::basic_ostream<char> &os) const;
	void Table(std::basic_ostream<char> &os, std::string const &nSpaceArg) const;

};

}
}

#include "configuration_section.h"
#include "configuration_access.h"
