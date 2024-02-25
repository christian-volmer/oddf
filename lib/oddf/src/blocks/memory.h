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

	Memory() implements a single-clock domain two-port memory with output
	register.

*/

#pragma once

namespace dfx {
namespace blocks {

template<typename T> class IMemoryBackdoor {

public:

	virtual void writeMemoryBackdoor(int address, T const *data, int count) = 0;
	virtual void readMemoryBackdoor(int address, T *data, int count) = 0;
};

#define DECLARE_MEMORY_FUNCTION(_type_) \
	node<_type_> InternalMemory(int size, node<dynfix> const &readAddress, node<bool> const &writeEnable, node<dynfix> const &writeAddress, node<_type_> const &writeData, IMemoryBackdoor<_type_> **memoryBackdoor); \
	bus<_type_> InternalMemory(int size, node<dynfix> const &readAddress, node<bool> const &writeEnable, node<dynfix> const &writeAddress, bus_access<_type_> const &writeData, IMemoryBackdoor<_type_> **memoryBackdoor);

DECLARE_MEMORY_FUNCTION(bool)
DECLARE_MEMORY_FUNCTION(double)
DECLARE_MEMORY_FUNCTION(std::int32_t)
DECLARE_MEMORY_FUNCTION(std::int64_t)
DECLARE_MEMORY_FUNCTION(dynfix)

#undef DECLARE_MEMORY_FUNCTION

template<typename T> bus<typename types::TypeTraits<T>::internalType> Memory(int size, node<dynfix> const &readAddress, node<bool> const &writeEnable, node<dynfix> const &writeAddress, bus_access<typename types::TypeTraits<T>::internalType> const &writeData, IMemoryBackdoor<typename types::TypeTraits<T>::internalType> *&backDoor)
{
	types::TypeDescription writeDataTypeDesc = types::GetDescription(writeData.first().GetDriver()->value);
	types::TypeDescription specifiedTypeDesc = types::GetDescription(T());

	if (writeDataTypeDesc != specifiedTypeDesc)
		throw design_error("Memory: Type of the 'WriteData' input ('" + writeDataTypeDesc.ToString() + "') must match the specified type of the memory ('" + specifiedTypeDesc.ToString() + "').");

	return InternalMemory(size, readAddress, writeEnable, writeAddress, writeData, &backDoor);
}

template<typename T> bus<typename types::TypeTraits<T>::internalType> Memory(int size, node<dynfix> const &readAddress, node<bool> const &writeEnable, node<dynfix> const &writeAddress, bus_access<typename types::TypeTraits<T>::internalType> const &writeData)
{
	types::TypeDescription writeDataTypeDesc = types::GetDescription(writeData.first().GetDriver()->value);
	types::TypeDescription specifiedTypeDesc = types::GetDescription(T());

	if (writeDataTypeDesc != specifiedTypeDesc)
		throw design_error("Memory: Type of the 'WriteData' input ('" + writeDataTypeDesc.ToString() + "') must match the specified type of the memory ('" + specifiedTypeDesc.ToString() + "').");

	return InternalMemory(size, readAddress, writeEnable, writeAddress, writeData, nullptr);
}

template<typename T> node<typename types::TypeTraits<T>::internalType> Memory(int size, node<dynfix> const &readAddress, node<bool> const &writeEnable, node<dynfix> const &writeAddress, node<typename types::TypeTraits<T>::internalType> const &writeData, IMemoryBackdoor<typename types::TypeTraits<T>::internalType> *&backDoor)
{
	return InternalMemory(size, readAddress, writeEnable, writeAddress, bus<typename types::TypeTraits<T>::internalType>(writeData), &backDoor).first();
}

template<typename T> node<typename types::TypeTraits<T>::internalType> Memory(int size, node<dynfix> const &readAddress, node<bool> const &writeEnable, node<dynfix> const &writeAddress, node<typename types::TypeTraits<T>::internalType> const &writeData)
{
	return InternalMemory(size, readAddress, writeEnable, writeAddress, bus<typename types::TypeTraits<T>::internalType>(writeData), nullptr).first();
}


}
}
