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

#include "../global.h"
#include "register_file.h"

namespace dfx {
namespace backend {
namespace blocks {

class write_register_block : public BlockBase {

private:

	OutputPin<dynfix> writeOutput;

	void Evaluate() override
	{
	}

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t();
	}

	bool CanEvaluate() const override
	{
		return true;
	}

public:

	write_register_block() :
		BlockBase("write_register"),
		writeOutput(this, ufix<64>())
	{
	}

	void Write(std::int64_t value)
	{
		writeOutput.value.data[0] = static_cast<std::int32_t>(value & 0xffffffff);
		writeOutput.value.data[1] = static_cast<std::int32_t>(value >> 32);
		SetDirty();
	}

	node<dynfix> GetNode() 
	{
		return writeOutput.GetNode();
	}
};

class read_register_block : public BlockBase {

private:

	InputPin<dynfix> readInput;

	void Evaluate() override
	{
	}

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t();
	}

	bool CanEvaluate() const override
	{
		return false;
	}

public:

	read_register_block(node<dynfix> const &source) :
		BlockBase("read_register"),
		readInput(this, source)
	{
	}

	std::int64_t Read() const
	{
		dynfix const &value = readInput.GetValue();
		return (static_cast<std::int64_t>(value.data[0]) & 0xffffffff) | (static_cast<std::int64_t>(value.data[1]) << 32);
	}
};

}
}

namespace modules {

//
// RegisterFile
//

RegisterFile::register_description_base::register_description_base() :
	name(),
	fullName(),
	instance(nullptr),
	typeDesc(),
	address(0),
	arrayLength(0),
	arrayIndex(0)
{
}

RegisterFile::read_register_description::read_register_description() :
	register_description_base(),
	readBlock(nullptr)
{
}

RegisterFile::write_register_description::write_register_description() :
	register_description_base(),
	writeBlock(nullptr)
{
}

RegisterFile::RegisterFile() :
	baseHierarchy(Design::GetCurrent().GetHierarchy().GetCurrentLevel()),
	ReadRegisterBus(nullptr),
	ReadNames(),
	ReadDefinitions(),
	WriteRegisterBus(nullptr),
	WriteNames(),
	WriteDefinitions()
{
}

void RegisterFile::SetReadRegisterBus(bus<dynfix> &readRegisterBus)
{
	if (ReadRegisterBus != nullptr)
		throw design_error("A read register bus has already been set for this register file.");
	ReadRegisterBus = &readRegisterBus;
}

void RegisterFile::SetWriteRegisterBus(forward_bus<ufix<64>> &writeRegisterBus)
{
	if (WriteRegisterBus != nullptr)
		throw design_error("A write register bus has already been set for this register file.");
	WriteRegisterBus = &writeRegisterBus;
}

void RegisterFile::report(std::basic_ostream<char> &os) const
{
	using std::setw;
	using std::endl;

	os << endl << " --- Write Registers --- " << endl;

	HierarchyLevel const *previous_instance = nullptr;

	for (auto const &reg : WriteDefinitions) {

		if (reg.arrayIndex > 0)
			continue;

		HierarchyLevel const *new_instance = reg.instance;

		if (new_instance != previous_instance)
			os << endl << " " << new_instance->GetFullName() << endl << endl;

		previous_instance = new_instance;

		std::string description = reg.description.empty() ? "<no description>" : reg.description;

		if (reg.arrayLength == 1)
			os << setw(6) << reg.address << setw(40) << reg.name << setw(30) << reg.typeDesc.ToString() << "       " << description << endl;
		else
			os << setw(6) << reg.address << setw(40) << reg.name << setw(30) << string_printf("%s[%d]", reg.typeDesc.ToString().c_str(), reg.arrayLength) << "       " << description << endl;
	}

	os << endl;

	os << endl << " --- Read Registers --- " << endl;

	previous_instance = nullptr;

	for (auto const &reg : ReadDefinitions) {

		if (reg.arrayIndex > 0)
			continue;

		HierarchyLevel const *new_instance = reg.instance;

		if (new_instance != previous_instance)
			os << endl << " " << new_instance->GetFullName() << endl << endl;

		previous_instance = new_instance;

		std::string description = reg.description.empty() ? "<no description>" : reg.description;

		if (reg.arrayLength == 1)
			os << setw(6) << reg.address << setw(40) << reg.name << setw(30) << reg.typeDesc.ToString() << "       " << description << endl;
		else
			os << setw(6) << reg.address << setw(40) << reg.name << setw(30) << string_printf("%s[%d]", reg.typeDesc.ToString().c_str(), reg.arrayLength) << "       " << description << endl;
	}

	os << endl;

}


//
// Write registers
//

void RegisterFile::InternalAddWriteRegister(std::string const &name, std::string const &description, types::TypeDescription const &typeDesc, int length, int index)
{
	if (WriteRegisterBus == nullptr)
		throw design_error("A write register bus must be set for this register file before write registers can be added.");

	write_register_description regDesc;
	regDesc.name = name;
	regDesc.instance = Design::GetCurrent().GetHierarchy().GetCurrentLevel();
	regDesc.fullName = regDesc.instance->GetFullName() + "/" + regDesc.name;
	regDesc.description = description;
	regDesc.address = (int)WriteDefinitions.size();
	regDesc.typeDesc = typeDesc;
	regDesc.arrayLength = length;
	regDesc.arrayIndex = index;

	{
		ScopedGotoHierarchyLevel _level(baseHierarchy);
		regDesc.writeBlock = &Design::GetCurrent().NewBlock<backend::blocks::write_register_block>();
	}
	Outputs.WriteRegisters.append(regDesc.writeBlock->GetNode());
	WriteDefinitions.push_back(regDesc);

	if (index == 0) {

		if (WriteNames.find(regDesc.fullName) != WriteNames.end())
			throw design_error("A write register named '" + regDesc.fullName + "' already exists.");

		if (ReadNames.find(regDesc.fullName) != ReadNames.end())
			throw design_error("A read register named '" + regDesc.fullName + "' already exists.");

		WriteNames[regDesc.fullName] = regDesc.address;
	}
}

RegisterFile::write_register_description const &RegisterFile::WriteResolveName(std::string const &name) const
{
	auto const &addressIt = WriteNames.find(name);

	if (addressIt == WriteNames.end())
		throw design_error("There is no such write register named '" + name + "'.");

	return WriteDefinitions.at(addressIt->second);
}

void RegisterFile::ClearAll()
{
	for (auto &def : WriteDefinitions)
		def.writeBlock->Write(0);
}

void RegisterFile::WriteRaw(int address, std::int64_t rawValue)
{
	WriteDefinitions.at(address).writeBlock->Write(rawValue);
}

void RegisterFile::InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, std::int32_t const *values)
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Int32:
		case types::TypeDescription::Int64: {

			for (int i = 0; i < count; ++i) {

				WriteDefinitions.at(startAddress + i).writeBlock->Write(*values);
				++values;
			}

			break;
		}

		case types::TypeDescription::Double: {

			for (int i = 0; i < count; ++i) {

				double doubleValue = *values;
				std::int64_t value64 = reinterpret_cast<std::int64_t const &>(doubleValue);
				WriteDefinitions.at(startAddress + i).writeBlock->Write(value64);
				++values;
			}

			break;
		}

		case types::TypeDescription::FixedPoint: {

			dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());

			for (int i = 0; i < count; ++i) {

				dynfix valueDynFix(*values);

				int align = typeDesc.GetFraction() - valueDynFix.GetFraction();

				if (align >= 0)
					valueDynFix.CopyShiftLeft(temp, align);
				else
					valueDynFix.CopyShiftRight(temp, -align);

				temp.OverflowWrapAround();

				std::int64_t value64 = (static_cast<std::int64_t>(temp.data[0]) & 0xffffffff) | (static_cast<std::int64_t>(temp.data[1]) << 32);
				WriteDefinitions.at(startAddress + i).writeBlock->Write(value64);
				++values;
			}

			break;
		}


		default:
			throw std::bad_cast();
	}
}

void RegisterFile::InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, std::int64_t const *values)
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Int32:
		case types::TypeDescription::Int64: {

			for (int i = 0; i < count; ++i) {

				WriteDefinitions.at(startAddress + i).writeBlock->Write(*values);
				++values;
			}

			break;
		}

		case types::TypeDescription::Double: {

			for (int i = 0; i < count; ++i) {

				double doubleValue = static_cast<double>(*values);
				std::int64_t value64 = reinterpret_cast<std::int64_t const &>(doubleValue);
				WriteDefinitions.at(startAddress + i).writeBlock->Write(value64);
				++values;
			}

			break;
		}

		case types::TypeDescription::FixedPoint: {

			dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());

			for (int i = 0; i < count; ++i) {

				dynfix valueDynFix(*values);

				int align = typeDesc.GetFraction() - valueDynFix.GetFraction();

				if (align >= 0)
					valueDynFix.CopyShiftLeft(temp, align);
				else
					valueDynFix.CopyShiftRight(temp, -align);

				temp.OverflowWrapAround();

				std::int64_t value64 = (static_cast<std::int64_t>(temp.data[0]) & 0xffffffff) | (static_cast<std::int64_t>(temp.data[1]) << 32);
				WriteDefinitions.at(startAddress + i).writeBlock->Write(value64);
				++values;
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}

void RegisterFile::InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, double const *values)
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Double: {

			for (int i = 0; i < count; ++i) {

				std::int64_t value64 = reinterpret_cast<std::int64_t const &>(*values);
				WriteDefinitions.at(startAddress + i).writeBlock->Write(value64);
				++values;
			}

			break;
		}

		case types::TypeDescription::FixedPoint: {

			dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());

			for (int i = 0; i < count; ++i) {

				dynfix valueDynFix(*values);

				int align = typeDesc.GetFraction() - valueDynFix.GetFraction();

				if (align >= 0)
					valueDynFix.CopyShiftLeft(temp, align);
				else
					valueDynFix.CopyShiftRight(temp, -align);

				temp.OverflowWrapAround();

				std::int64_t value64 = (static_cast<std::int64_t>(temp.data[0]) & 0xffffffff) | (static_cast<std::int64_t>(temp.data[1]) << 32);
				WriteDefinitions.at(startAddress + i).writeBlock->Write(value64);
				++values;
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}

void RegisterFile::InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, bool const *values)
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Boolean: {

			for (int i = 0; i < count; ++i) {

				WriteDefinitions.at(startAddress + i).writeBlock->Write(*values);
				++values;
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}

//
// Read registers
//

void RegisterFile::InternalAddReadRegister(std::string const &name, std::string const &description, types::TypeDescription const &typeDesc, int length, int index)
{
	if (ReadRegisterBus == nullptr)
		throw design_error("A read register bus must be set for this register file before read registers can be added.");

	read_register_description regDesc;
	regDesc.name = name;
	regDesc.instance = Design::GetCurrent().GetHierarchy().GetCurrentLevel();
	regDesc.fullName = regDesc.instance->GetFullName() + "/" + regDesc.name;
	regDesc.description = description;
	regDesc.address = (int)ReadDefinitions.size();
	regDesc.typeDesc = typeDesc;
	regDesc.arrayLength = length;
	regDesc.arrayIndex = index;

	{
		ScopedGotoHierarchyLevel _level(baseHierarchy);
		regDesc.readBlock = &Design::GetCurrent().NewBlock<backend::blocks::read_register_block>(Inputs.ReadRegisters.append());
	}

	ReadDefinitions.push_back(regDesc);

	if (index == 0) {

		if (WriteNames.find(regDesc.fullName) != WriteNames.end())
			throw design_error("A write register named '" + regDesc.fullName + "' already exists.");

		if (ReadNames.find(regDesc.fullName) != ReadNames.end())
			throw design_error("A read register named '" + regDesc.fullName + "' already exists.");

		ReadNames[regDesc.fullName] = regDesc.address;
	}
}

RegisterFile::read_register_description const &RegisterFile::ReadResolveName(std::string const &name) const
{
	auto const &addressIt = ReadNames.find(name);

	if (addressIt == ReadNames.end())
		throw design_error("There is no such read register named '" + name + "'.");

	return ReadDefinitions.at(addressIt->second);
}

std::int64_t RegisterFile::ReadRaw(int address) const
{
	return ReadDefinitions.at(address).readBlock->Read();
}

void RegisterFile::InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, std::int32_t *values) const
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Int32: {

			for (int i = 0; i < count; ++i) {

				*values = static_cast<std::int32_t>(ReadDefinitions.at(startAddress + i).readBlock->Read());
				++values;
			}

			break;
		}

		case types::TypeDescription::FixedPoint: {

			if (typeDesc.GetFraction() == 0 && ((typeDesc.IsSigned() && typeDesc.GetWordWidth() <= 32) || (!typeDesc.IsSigned() && typeDesc.GetWordWidth() <= 31))) {

				for (int i = 0; i < count; ++i) {

					*values = static_cast<std::int32_t>(ReadDefinitions.at(startAddress + i).readBlock->Read());
					++values;
				}
			}
			else
				throw std::bad_cast();

			break;
		}

		default:
			throw std::bad_cast();
	}
}

void RegisterFile::InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, std::int64_t *values) const
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Int64: {

			for (int i = 0; i < count; ++i) {

				*values = ReadDefinitions.at(startAddress + i).readBlock->Read();
				++values;
			}

			break;
		}

		case types::TypeDescription::FixedPoint: {

			if (typeDesc.GetFraction() == 0 && ((typeDesc.IsSigned() && typeDesc.GetWordWidth() <= 64) || (!typeDesc.IsSigned() && typeDesc.GetWordWidth() <= 63))) {

				for (int i = 0; i < count; ++i) {

					*values = ReadDefinitions.at(startAddress + i).readBlock->Read();
					++values;
				}
			}
			else
				throw std::bad_cast();
				
			break;
		}

		default:
			throw std::bad_cast();
	}
}

void RegisterFile::InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, double *values) const
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Double: {

			for (int i = 0; i < count; ++i) {

				std::int64_t value64 = ReadDefinitions.at(startAddress + i).readBlock->Read();
				*values = reinterpret_cast<double const &>(value64);
				++values;
			}

			break;
		}

		case types::TypeDescription::FixedPoint: {

			for (int i = 0; i < count; ++i) {

				std::int64_t value64 = ReadDefinitions.at(startAddress + i).readBlock->Read();

				dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());
				temp.data[0] = (std::int32_t)(value64 & 0xffffffff);
				temp.data[1] = (std::int32_t)((value64 >> 32) & 0xffffffff);
				temp.OverflowWrapAround();

				*values = static_cast<double>(temp);
				++values;
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}

void RegisterFile::InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, bool *values) const
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Boolean: {

			for (int i = 0; i < count; ++i) {

				*values = ReadDefinitions.at(startAddress + i).readBlock->Read() != 0;
				++values;
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}


}
}
