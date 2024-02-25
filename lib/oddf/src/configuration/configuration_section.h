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

class Section {

private:

	IBuilder &builder;

	template<typename T> struct TypeSupport {

		static bool const isSupported = false;
		static int const isSigned = false;
		static int const wordWidth = 0;
		static int const fraction = 0;
		using DynfixT = void;
	};

	int extraPipelining;

public:

	Section(IBuilder &theBuilder) : builder(theBuilder), extraPipelining(0)
	{
		builder.Break(0);
	}

	Section(IBuilder &theBuilder, int extraPipelining) : builder(theBuilder), extraPipelining(0)
	{
		builder.Break(extraPipelining);
	}

	Section() = delete;
	void operator=(Section const &) = delete;

	~Section()
	{
		builder.Merge(extraPipelining);
	}

	void SetExtraPipelining(int theExtraPipelining)
	{
		extraPipelining = theExtraPipelining;
	}

	// Adds a write register with scalar data type.
	template<typename T> typename std::enable_if<std::rank<T>::value == 0, node<typename types::TypeTraits<T>::internalType>>::type AddWriteRegister(std::string const &name, std::string const &description);

	// Adds a write register with array data type.
	template<typename T> typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), bus<typename types::TypeTraits<typename std::remove_all_extents<T>::type>::internalType>>::type AddWriteRegister(std::string const &name, std::string const &description);

	// Adds a read register with scalar data type.
	template<typename T> typename std::enable_if<std::rank<T>::value == 0, forward_node<T>>::type AddReadRegister(std::string const &name, std::string const &description);
	void AddReadRegister(std::string const &name, std::string const &description, node<dynfix> const &source);
	void AddReadRegister(std::string const &name, std::string const &description, node<bool> const &source);

	// Adds a read register with array data type.
	template<typename T> typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), forward_bus<typename std::remove_all_extents<T>::type>>::type AddReadRegister(std::string const &name, std::string const &description);

	// Adds a range.
	template<typename T> configuration::Range AddRange(std::string const &name, std::string const &description, int length);
};

template<int wordWidthArg, int fractionArg> struct Section::TypeSupport<ufix<wordWidthArg, fractionArg>> {

	static bool const isSupported = true;
	static int const isSigned = false;
	static int const wordWidth = wordWidthArg;
	static int const fraction = fractionArg;
	using DynfixT = ufix<wordWidth, fraction>;
};

template<int wordWidthArg, int fractionArg> struct Section::TypeSupport<sfix<wordWidthArg, fractionArg>> {

	static bool const isSupported = true;
	static int const isSigned = true;
	static int const wordWidth = wordWidthArg;
	static int const fraction = fractionArg;
	using DynfixT = sfix<wordWidth, fraction>;
};

template<> struct Section::TypeSupport<bool> {

	static bool const isSupported = true;
	static int const isSigned = false;
	static int const wordWidth = 1;
	static int const fraction = 0;
	using DynfixT = ufix<1, 0>;
};

template<> struct Section::TypeSupport<std::int32_t> {

	static bool const isSupported = true;
	static int const isSigned = true;
	static int const wordWidth = 32;
	static int const fraction = 0;
	using DynfixT = sfix<32, 0>;
};

template<> struct Section::TypeSupport<std::int64_t> {

	static bool const isSupported = true;
	static int const isSigned = true;
	static int const wordWidth = 64;
	static int const fraction = 0;
	using DynfixT = sfix<64, 0>;
};

template<> struct Section::TypeSupport<double> {

	static bool const isSupported = true;
	static int const isSigned = true;
	static int const wordWidth = 64;
	static int const fraction = 0;
	using DynfixT = sfix<64, 0>;
};


//
// AddWriteRegister API
//

template<typename T> 
typename std::enable_if<std::rank<T>::value == 0, node<typename types::TypeTraits<T>::internalType>>::type Section::AddWriteRegister(std::string const &name, std::string const &description)
{
	static_assert(TypeSupport<T>::isSupported, "The given type is not supported as a register.");

	builder.GetNamespace().AddEntry(
		Namespace::Entry::Access::READ_WRITE,
		name,
		description,
		Design::GetCurrent().GetHierarchy().GetCurrentLevel(),
		types::GetDescription(T()),
		builder.GetCurrentAddress(),
		1);

	return blocks::ReinterpretCast<T>(builder.AddRegister(TypeSupport<T>::isSigned, TypeSupport<T>::wordWidth, TypeSupport<T>::fraction, "Cfg_" + name));
}

template<typename T> 
typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), bus<typename types::TypeTraits<typename std::remove_all_extents<T>::type>::internalType>>::type Section::AddWriteRegister(std::string const &name, std::string const &description)
{
	using baseT = typename std::decay<typename std::remove_all_extents<T>::type>::type;
	int const width = std::extent<T>::value;

	static_assert(TypeSupport<baseT>::isSupported, "The given type is not supported as a register.");

	builder.GetNamespace().AddEntry(
		Namespace::Entry::Access::READ_WRITE,
		name,
		description,
		Design::GetCurrent().GetHierarchy().GetCurrentLevel(),
		types::GetDescription(baseT()),
		builder.GetCurrentAddress(),
		width);

	bus<dynfix> busRegister;
	for (int i = 0; i < width; ++i)
		busRegister.append(builder.AddRegister(TypeSupport<baseT>::isSigned, TypeSupport<baseT>::wordWidth, TypeSupport<baseT>::fraction, "Cfg_" + name + "_" + std::to_string(i)));

	return blocks::ReinterpretCast<baseT>(busRegister);
}


//
// AddReadRegister API
//

template<typename T>
inline typename std::enable_if<std::rank<T>::value == 0, forward_node<T>>::type Section::AddReadRegister(std::string const &name, std::string const &description)
{
	static_assert(TypeSupport<T>::isSupported, "The given type is not supported as a register.");

	builder.GetNamespace().AddEntry(
		Namespace::Entry::Access::READ_ONLY,
		name,
		description,
		Design::GetCurrent().GetHierarchy().GetCurrentLevel(),
		types::GetDescription(T()),
		builder.GetCurrentAddress(),
		1);

	forward_node<T> userNode;
	builder.AddObserver(blocks::ReinterpretCast<typename TypeSupport<T>::DynfixT>(userNode), "Obs_" + name);
	return userNode;
}

inline void Section::AddReadRegister(std::string const &name, std::string const &description, node<dynfix> const &source)
{
	builder.GetNamespace().AddEntry(
		Namespace::Entry::Access::READ_ONLY,
		name,
		description,
		Design::GetCurrent().GetHierarchy().GetCurrentLevel(),
		source.GetType(),
		builder.GetCurrentAddress(),
		1);

	builder.AddObserver(source, "Obs_" + name);
}

inline void Section::AddReadRegister(std::string const &name, std::string const &description, node<bool> const &source)
{
	builder.GetNamespace().AddEntry(
		Namespace::Entry::Access::READ_ONLY,
		name,
		description,
		Design::GetCurrent().GetHierarchy().GetCurrentLevel(),
		source.GetType(),
		builder.GetCurrentAddress(),
		1);

	builder.AddObserver(blocks::ReinterpretCast<typename TypeSupport<bool>::DynfixT>(source), "Obs_" + name);
}

// Adds a read register with array data type.
template<typename T>
inline typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), forward_bus<typename std::remove_all_extents<T>::type>>::type Section::AddReadRegister(std::string const &name, std::string const &description)
{
	using baseT = typename std::decay<typename std::remove_all_extents<T>::type>::type;
	int const width = std::extent<T>::value;

	static_assert(TypeSupport<baseT>::isSupported, "The given type is not supported as a register.");

	builder.GetNamespace().AddEntry(
		Namespace::Entry::Access::READ_ONLY,
		name,
		description,
		Design::GetCurrent().GetHierarchy().GetCurrentLevel(),
		types::GetDescription(baseT()),
		builder.GetCurrentAddress(),
		width);

	forward_bus<baseT> userBus(width);
	for (int i = 0; i < width; ++i)
		builder.AddObserver(blocks::ReinterpretCast<typename TypeSupport<baseT>::DynfixT>(userBus[i]), "Obs_" + name + "_" + std::to_string(i));
	return userBus;
}


//
// AddRange API
//

template<typename T> configuration::Range Section::AddRange(std::string const &name, std::string const &description, int length)
{
	static_assert(TypeSupport<T>::isSupported, "The given type is not supported as a register.");

	// TODO: To support types other than dynfix (even bool), we need to extend the configuration::Range return type.
	static_assert(std::is_same<typename TypeSupport<T>::DynfixT, T>::value, "The given type is not supported as a register.");

	builder.GetNamespace().AddEntry(
		Namespace::Entry::Access::READ_WRITE,
		name,
		description,
		Design::GetCurrent().GetHierarchy().GetCurrentLevel(),
		types::GetDescription(T()),
		builder.GetCurrentAddress(),
		length);

	return builder.AddRange(TypeSupport<T>::isSigned, TypeSupport<T>::wordWidth, TypeSupport<T>::fraction, length, name);
}


}
}
