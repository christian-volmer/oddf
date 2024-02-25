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

	Obsolete, will be removed eventually. Use SerialConfigurationBuilder
	and SerialConfigurationController instead.

*/

#pragma once

namespace dfx {
namespace backend {
namespace blocks {

class write_register_block; 
class read_register_block;

}
}

namespace modules {

class RegisterFile {

private:

	struct register_description_base {

		std::string name;
		std::string fullName;
		std::string description;
		HierarchyLevel const *instance;
		types::TypeDescription typeDesc;
		int address;
		int arrayLength;
		int arrayIndex;

		register_description_base();
	};

	HierarchyLevel *baseHierarchy;

	//
	// Read registers

	struct read_register_description : public register_description_base {

		backend::blocks::read_register_block *readBlock;

		read_register_description();
	};

	bus<dynfix> *ReadRegisterBus;

	std::unordered_map<std::string, int> ReadNames;
	std::vector<read_register_description> ReadDefinitions;

	void InternalAddReadRegister(std::string const &name, std::string const &description, types::TypeDescription const &typeDesc, int length, int index);

	// Read a register through a standard type
	void InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, std::int32_t *values) const;
	void InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, std::int64_t *values) const;
	void InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, double *values) const;
	void InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, bool *values) const;

	read_register_description const &ReadResolveName(std::string const &name) const;


	//
	// Write registers 

	struct write_register_description : public register_description_base {

		backend::blocks::write_register_block *writeBlock;

		write_register_description();
	};

	forward_bus<ufix<64>> *WriteRegisterBus;

	std::unordered_map<std::string, int> WriteNames;
	std::vector<write_register_description> WriteDefinitions;

	void InternalAddWriteRegister(std::string const &name, std::string const &description, types::TypeDescription const &typeDesc, int length, int index);

	// Write a register through a standard type
	void InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, std::int32_t const *values);
	void InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, std::int64_t const *values);
	void InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, double const *values);
	void InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, bool const *values);

	write_register_description const &WriteResolveName(std::string const &name) const;

public:

	struct {

		inout::input_bus<ufix<64>> ReadRegisters;

	} Inputs;

	struct {

		inout::output_bus<dynfix> WriteRegisters;

	} Outputs;

	RegisterFile();
	RegisterFile(RegisterFile const &) = delete;

	RegisterFile &operator =(RegisterFile const &) = delete;

	void report(std::basic_ostream<char> &os) const;

	//
	// Read registers

	void SetReadRegisterBus(bus<dynfix> &readRegisterBus);

	// Adds a read register with scalar data type.
	template<typename T> typename std::enable_if<std::rank<T>::value == 0, forward_node<T>>::type AddReadRegister(std::string const &name, std::string const &description);

	// Adds a read register with array data type.
	template<typename T> typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), forward_bus<typename std::remove_all_extents<T>::type>>::type AddReadRegister(std::string const &name, std::string const &description);


	//
	// Read access

	// Reads the raw register value from the given address.
	std::int64_t ReadRaw(int address) const;

	template<typename T> typename std::enable_if<std::rank<T>::value == 0, T>::type Read(std::string const &name) const;
	template<typename T> typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), void>::type Read(std::string const &name, T &values) const;


	//
	// Write registers

	void SetWriteRegisterBus(forward_bus<ufix<64>> &writeRegisterBus);

	// Adds a write register with scalar data type.
	template<typename T> typename std::enable_if<std::rank<T>::value == 0, node<typename types::TypeTraits<T>::internalType>>::type AddWriteRegister(std::string const &name, std::string const &description);

	// Adds a write register with array data type.
	template<typename T> typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), bus<typename types::TypeTraits<typename std::remove_all_extents<T>::type>::internalType>>::type AddWriteRegister(std::string const &name, std::string const &description);

	//
	// Write access

	// Sets all write registers to zero
	void ClearAll();

	// Write a raw register value to the given address.
	void WriteRaw(int address, std::int64_t rawValue);

	template<typename T> typename std::enable_if<std::rank<T>::value == 0, void>::type Write(std::string const &name, T const &value);
	template<typename T> typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), void>::type Write(std::string const &name, T const &values);
	template<typename T> void Write(std::string const &name, std::initializer_list<T> values);

};


//
// Write registers high-level API
//

template<typename T>
inline typename std::enable_if<std::rank<T>::value == 0, node<typename types::TypeTraits<T>::internalType>>::type RegisterFile::AddWriteRegister(std::string const &name, std::string const &description)
{
	types::TypeDescription typeDesc = types::GetDescription(T());

	if (!types::IsInitialised(T()))
		throw design_error("The type '" + typeDesc.ToString() + "' cannot be used for a register.");

	InternalAddWriteRegister(name, description, typeDesc, 1, 0);

	node<dynfix> writeRegInput = WriteRegisterBus->append();
	DFX_INPUT_NAME(writeRegInput, "WriteReg_" + name);

	return blocks::ReinterpretCast<T>(writeRegInput);
}

template<typename T>
inline typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), bus<typename types::TypeTraits<typename std::remove_all_extents<T>::type>::internalType>>::type RegisterFile::AddWriteRegister(std::string const &name, std::string const &description)
{
	using baseT = typename std::remove_all_extents<T>::type;
	int const width = std::extent<T>::value;

	types::TypeDescription typeDesc = types::GetDescription(baseT());

	if (!types::IsInitialised(baseT()))
		throw design_error("The type '" + typeDesc.ToString() + "' cannot be used for a register.");

	for (int i = 0; i < width; ++i)
		InternalAddWriteRegister(name, description, typeDesc, width, i);

	bus<dynfix> writeRegInput = WriteRegisterBus->append(width);
	DFX_INPUT_NAME(writeRegInput, "WriteReg_" + name);

	return blocks::ReinterpretCast<baseT>(writeRegInput);
}

template<typename T>
inline typename std::enable_if<std::rank<T>::value == 0, void>::type RegisterFile::Write(std::string const &name, T const &value)
{
	auto const &regDesc = WriteResolveName(name);
	try {

		// TODO: catch case of writing an array register
		InternalWrite(regDesc.address, 1, regDesc.typeDesc, &value);
	}
	catch (std::bad_cast const &) {

		throw design_error("Write register '" + regDesc.fullName + "' is of type '" + regDesc.typeDesc.ToString() + "'. Cannot write it through type '" + types::GetDescription(T()).ToString() + "'.");
	}
}

template<typename T>
inline typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), void>::type RegisterFile::Write(std::string const &name, T const &values)
{
	using baseT = typename std::decay<typename std::remove_all_extents<T>::type>::type;
	int const width = std::extent<T>::value;

	auto const &regDesc = WriteResolveName(name);

	try {

		if (width != regDesc.arrayLength)
			throw std::bad_cast("Array dimensions do not match.");

		InternalWrite(regDesc.address, width, regDesc.typeDesc, values);
	}
	catch (std::bad_cast const &) {

		throw design_error("Write register '" + regDesc.fullName + "' is of type '"
						   + string_printf("%s[%d]", regDesc.typeDesc.ToString().c_str(), regDesc.arrayLength)
						   + "'. Cannot write it through type '"
						   + string_printf("%s[%d]", types::GetDescription(baseT()).ToString().c_str(), width) + "'.");
	}
}


template<typename T>
inline void RegisterFile::Write(std::string const &name, std::initializer_list<T> values)
{
	int const width = (int)values.size();
	auto const &regDesc = WriteResolveName(name);

	try {

		if (width != regDesc.arrayLength)
			throw std::bad_cast("Array dimensions do not match.");

		int i = 0;
		for (auto const &value : values) {

			InternalWrite(regDesc.address + i, 1, regDesc.typeDesc, &value);
			++i;
		}
	}
	catch (std::bad_cast const &) {

		throw design_error("Write register '" + regDesc.fullName + "' is of type '"
						   + string_printf("%s[%d]", regDesc.typeDesc.ToString().c_str(), regDesc.arrayLength)
						   + "'. Cannot write it through type '"
						   + string_printf("%s[%d]", types::GetDescription(T()).ToString().c_str(), width) + "'.");
	}
}


//
// Read registers high-level API
//

template<typename T>
inline typename std::enable_if<std::rank<T>::value == 0, forward_node<T>>::type RegisterFile::AddReadRegister(std::string const &name, std::string const &description)
{
	types::TypeDescription typeDesc = types::GetDescription(T());

	if (!types::IsInitialised(T()))
		throw design_error("The type '" + typeDesc.ToString() + "' cannot be used for a register.");

	InternalAddReadRegister(name, description, typeDesc, 1, 0);

	forward_node<T> userNode;
	node<dynfix> readRegOutput = blocks::ReinterpretCast<ufix<64>>(userNode);
	ReadRegisterBus->append(readRegOutput);
	DFX_OUTPUT_NAME(readRegOutput, "ReadReg_" + name);
	return userNode;
}


// Adds a read register with array data type.
template<typename T> 
inline typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), forward_bus<typename std::remove_all_extents<T>::type>>::type RegisterFile::AddReadRegister(std::string const &name, std::string const &description)
{
	using baseT = typename std::remove_all_extents<T>::type;
	int const width = std::extent<T>::value;

	types::TypeDescription typeDesc = types::GetDescription(baseT());

	if (!types::IsInitialised(baseT()))
		throw design_error("The type '" + typeDesc.ToString() + "' cannot be used for a register.");


	for (int i = 0; i < width; ++i)
		InternalAddReadRegister(name, description, typeDesc, width, i);

	forward_bus<baseT> userBus(width);
	bus<dynfix> readRegOutput = blocks::ReinterpretCast<ufix<64>>(userBus);
	ReadRegisterBus->append(readRegOutput);
	DFX_OUTPUT_NAME(readRegOutput, "ReadReg_" + name);
	return userBus;
}

template<typename T> 
inline typename std::enable_if<std::rank<T>::value == 0, T>::type RegisterFile::Read(std::string const &name) const
{
	auto const &regDesc = ReadResolveName(name);
	try {

		// TODO: catch case of reading an array register

		T value;
		InternalRead(regDesc.address, 1, regDesc.typeDesc, &value);
		return value;
	}
	catch (std::bad_cast const &) {

		throw design_error("Read register '" + regDesc.fullName + "' is of type '" + regDesc.typeDesc.ToString() + "'. Cannot read it through type '" + types::GetDescription(T()).ToString() + "'.");
	}
}


template<typename T>
inline typename std::enable_if<(std::rank<T>::value == 1) && (std::extent<T>::value > 0), void>::type RegisterFile::Read(std::string const &name, T &values) const
{
	using baseT = typename std::decay<typename std::remove_all_extents<T>::type>::type;
	int const width = std::extent<T>::value;

	auto const &regDesc = ReadResolveName(name);

	try {

		if (width != regDesc.arrayLength)
			throw std::bad_cast("Array dimensions do not match.");

		InternalRead(regDesc.address, width, regDesc.typeDesc, values);
	}
	catch (std::bad_cast const &) {

		throw design_error("Read register '" + regDesc.fullName + "' is of type '"
						   + string_printf("%s[%d]", regDesc.typeDesc.ToString().c_str(), regDesc.arrayLength)
						   + "'. Cannot read it through type '"
						   + string_printf("%s[%d]", types::GetDescription(baseT()).ToString().c_str(), width) + "'.");
	}
}
}
}
