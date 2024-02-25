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

#include <string>
#include <unordered_map>
#include <vector>

#include "../node.h"

namespace dfx {
namespace configuration {


//
// Section
//

class Access {

private:

	configuration::IController &configController;
	configuration::Namespace &configNamespace;

	// Read a register through a standard type
	void InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, std::int32_t *values) const;
	void InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, std::int64_t *values) const;
	void InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, double *values) const;
	void InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, bool *values) const;

	// Write a register through a standard type
	void InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, std::int32_t const *values);
	void InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, std::int64_t const *values);
	void InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, double const *values);
	void InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, bool const *values);


public:

	Access(configuration::IController &theController, configuration::Namespace &theNamespace);

	Access() = delete;
	Access(Access const &) = delete;
	void operator=(Access const &) = delete;

	template<typename T> typename std::enable_if<std::rank<T>::value == 0, T>::type Read(std::string const &name) const;
	template<typename T> typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), void>::type Read(std::string const &name, T &values, int offset = 0) const;

	template<typename T> typename std::enable_if<std::rank<T>::value == 0, void>::type Write(std::string const &name, T const &value);
	template<typename T> typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), void>::type Write(std::string const &name, T const &values, int offset = 0);
	template<typename iteratorT> void Write(std::string const &name, iteratorT first, iteratorT last, int offset = 0);
	template<typename T> void Write(std::string const &name, std::initializer_list<T> values, int offset = 0);

	void ClearAll();

	configuration::IController &GetController();
};


//
// high-level write access
//

template<typename T>
inline typename std::enable_if<std::rank<T>::value == 0, void>::type Access::Write(std::string const &name, T const &value)
{
	auto const *regDesc = configNamespace.Lookup(name);
	if (!regDesc)
		throw design_error("There is no element with name '" + name + "'.");

	try {

		// TODO: catch case of writing an array register
		InternalWrite(regDesc->address, 1, regDesc->typeDesc, &value);
	}
	catch (std::bad_cast const &) {

		throw design_error("Element '" + regDesc->fullName + "' is of type '" + regDesc->typeDesc.ToString() + "'. Cannot write it through type '" + types::GetDescription(T()).ToString() + "'.");
	}
}

template<typename T>
inline typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), void>::type Access::Write(std::string const &name, T const &values, int offset /* = 0 */)
{
	using baseT = typename std::decay<typename std::remove_all_extents<T>::type>::type;
	int const width = std::extent<T>::value;

	auto const *regDesc = configNamespace.Lookup(name);
	if (!regDesc)
		throw design_error("There is no element with name '" + name + "'.");

	try {

		if (offset < 0 || offset + width > regDesc->arrayLength)
			throw design_error("Cannot access elements outside the bounds of this array register or address range.");

		InternalWrite(regDesc->address + offset, width, regDesc->typeDesc, values);
	}
	catch (std::bad_cast const &) {

		throw design_error("Element '" + regDesc->fullName + "' is of type '" + regDesc->typeDesc.ToString() + "'. Cannot write it through type '" + types::GetDescription(baseT()).ToString() + "'.");
	}
}

template<typename iteratorT> 
void Access::Write(std::string const &name, iteratorT first, iteratorT last, int offset /* = 0 */)
{
	using baseT = typename std::iterator_traits<iteratorT>::value_type;
	int const width = (int)std::distance(first, last);

	auto const *regDesc = configNamespace.Lookup(name);
	if (!regDesc)
		throw design_error("There is no element with name '" + name + "'.");

	try {

		if (width < 0 || offset < 0 || offset + width > regDesc->arrayLength)
			throw design_error("Cannot access elements outside the bounds of this array register or address range.");

		int currentAddress = regDesc->address + offset;
		while (first != last) {

			auto value = *first;
			InternalWrite(currentAddress, 1, regDesc->typeDesc, &value);
			++currentAddress;
			++first;
		}
	}
	catch (std::bad_cast const &) {

		throw design_error("Element '" + regDesc->fullName + "' is of type '" + regDesc->typeDesc.ToString() + "'. Cannot write it through type '" + types::GetDescription(baseT()).ToString() + "'.");
	}
}



template<typename T>
inline void Access::Write(std::string const &name, std::initializer_list<T> values, int offset /* = 0 */)
{
	int const width = (int)values.size();
	auto const *regDesc = configNamespace.Lookup(name);
	if (!regDesc)
		throw design_error("There is no element with name '" + name + "'.");

	try {

		if (offset < 0 || offset + width > regDesc->arrayLength)
			throw design_error("Cannot access elements outside the bounds of this array register or address range.");

		int i = 0;
		for (auto const &value : values) {

			InternalWrite(regDesc->address + i, 1, regDesc->typeDesc, &value);
			++i;
		}
	}
	catch (std::bad_cast const &) {

		throw design_error("Element '" + regDesc->fullName + "' is of type '" + regDesc->typeDesc.ToString() + "'. Cannot write it through type '" + types::GetDescription(T()).ToString() + "'.");
	}
}


//
// high-level read access
//


template<typename T>
inline typename std::enable_if<std::rank<T>::value == 0, T>::type Access::Read(std::string const &name) const
{
	auto const *regDesc = configNamespace.Lookup(name);
	if (!regDesc)
		throw design_error("There is no element with name '" + name + "'.");

	try {

		// TODO: catch case of reading an array register

		T value;
		InternalRead(regDesc->address, 1, regDesc->typeDesc, &value);
		return value;
	}
	catch (std::bad_cast const &) {

		throw design_error("Element '" + regDesc->fullName + "' is of type '" + regDesc->typeDesc.ToString() + "'. Cannot read it through type '" + types::GetDescription(T()).ToString() + "'.");
	}
}


template<typename T>
inline typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), void>::type Access::Read(std::string const &name, T &values, int offset /* = 0 */) const
{
	using baseT = typename std::decay<typename std::remove_all_extents<T>::type>::type;
	int const width = std::extent<T>::value;

	auto const *regDesc = configNamespace.Lookup(name);
	if (!regDesc)
		throw design_error("There is no element with name '" + name + "'.");

	try {

		if (offset < 0 || offset + width > regDesc->arrayLength)
			throw design_error("Cannot access elements outside the bounds of this array register or address range.");

		InternalRead(regDesc->address + offset, width, regDesc->typeDesc, values);
	}
	catch (std::bad_cast const &) {

		throw design_error("Element '" + regDesc->fullName + "' is of type '" + regDesc->typeDesc.ToString() + "'. Cannot read it through type '" + types::GetDescription(baseT()).ToString() + "'.");
	}
}

}
}
