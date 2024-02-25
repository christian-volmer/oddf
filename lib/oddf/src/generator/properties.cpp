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

	Classes that support the generation of code (e.g., Verilog) from the
	design. The 'Properties' class allows design blocks to expose named
	properties to the generator.

*/

#include "../global.h"

#include "properties.h"

namespace dfx {
namespace generator {

//
// Integer accessor functions
//

void Properties::SetInt(std::string const &name, int value)
{
	integerProperties[IndexedName(name, 0, 0)] = value;
}

void Properties::SetInt(std::string const &name, int index, int value)
{
	integerProperties[IndexedName(name, index, 0)] = value;
}

void Properties::SetInt(std::string const &name, int index, int index2, int value)
{
	integerProperties[IndexedName(name, index, index2)] = value;
}

int Properties::GetInt(std::string const &name) const
{
	return integerProperties.at(IndexedName(name, 0, 0));
}

int Properties::GetInt(std::string const &name, int index) const
{
	return integerProperties.at(IndexedName(name, index, 0));
}

int Properties::GetInt(std::string const &name, int index, int index2) const
{
	return integerProperties.at(IndexedName(name, index, index2));
}


//
// String accessor functions
//

void Properties::SetString(std::string const &name, std::string const &value)
{
	stringProperties[IndexedName(name, 0, 0)] = value;
}

void Properties::SetString(std::string const &name, int index, std::string const &value)
{
	stringProperties[IndexedName(name, index, 0)] = value;
}

std::string Properties::GetString(std::string const &name) const
{
	return stringProperties.at(IndexedName(name, 0, 0));
}

std::string Properties::GetString(std::string const &name, int index) const
{
	return stringProperties.at(IndexedName(name, index, 0));
}


//
// Comparison
//

bool Properties::operator ==(Properties const &rhs) const
{
	return integerProperties == rhs.integerProperties && stringProperties == rhs.stringProperties;

	/*
	if (integerProperties.size() != rhs.integerProperties.size())
		return false;

	if (stringProperties.size() != rhs.stringProperties.size())
		return false;

	for (auto &left : integerProperties) {

		auto right = rhs.integerProperties.find(left.first);
		if (right == rhs.integerProperties.end())
			return false;

		if (left.second != right->second)
			return false;
	}

	for (auto &left : stringProperties) {

		auto right = rhs.stringProperties.find(left.first);
		if (right == rhs.stringProperties.end())
			return false;

		if (left.second != right->second)
			return false;
	}

	return true;
	*/
}

bool Properties::operator !=(Properties const &rhs) const
{
	return !(*this == rhs);
}

std::size_t Properties::GetHash() const
{
	std::size_t hash = 0;

	for (auto &property : integerProperties) {

		hash_combine(hash, std::get<0>(property.first));
		hash_combine(hash, std::get<1>(property.first));
		hash_combine(hash, std::get<2>(property.first));
		hash_combine(hash, property.second);
	}

	for (auto &property : stringProperties) {

		hash_combine(hash, std::get<0>(property.first));
		hash_combine(hash, std::get<1>(property.first));
		hash_combine(hash, std::get<2>(property.first));
		hash_combine(hash, property.second);
	}

	return hash;
}

}
}
