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

    <no description>

*/

#pragma once

#include "ISimulatorComponentContext.h"
#include "SimulatorInstruction.h"

#include "Types.h"

#include <type_traits>
#include <cstdint>

namespace oddf::simulator::common::backend {

class ISimulatorCodeGenerationContext : public virtual ISimulatorComponentContext {

protected:

	virtual void InternalStartInstruction(size_t size, size_t alignment, SimulatorInstructionFunction<> *instructionFunction) = 0;
	virtual size_t InternalAddRecord(size_t size, size_t alignment) = 0;
	virtual void *InternalCommitInstruction() = 0;
	virtual void *InternalGetRecord(size_t offset) = 0;

public:

	virtual ~ISimulatorCodeGenerationContext() = default;

	template<typename instructionT>
	void StartInstruction(SimulatorInstructionFunction<instructionT> *instructionFunction)
	{
		static_assert(std::is_base_of_v<SimulatorInstruction, instructionT>);
		InternalStartInstruction(
			sizeof(instructionT),
			alignof(instructionT),
			reinterpret_cast<SimulatorInstructionFunction<> *>(instructionFunction));
	}

	template<typename instructionT, typename memberT>
	void StartInstructionVariadic(SimulatorInstructionFunction<instructionT> *instructionFunction,
		memberT (instructionT::*variaticMember)[1], size_t count)
	{
		static_assert(std::is_base_of_v<SimulatorInstruction, instructionT>);

		alignas(instructionT) char tempStorage[sizeof(instructionT)];
		size_t offset = reinterpret_cast<char const *>(&(reinterpret_cast<instructionT const *>(&tempStorage)->*variaticMember)) - tempStorage;

		// TODO: check, if possible, that the variadic member really is the last member.

		size_t size = offset + count * sizeof(memberT);

		// Round up to the next multiple of alignof(instructionT)
		size = ((size + alignof(instructionT) - 1) / alignof(instructionT)) * alignof(instructionT);

		InternalStartInstruction(
			size,
			alignof(instructionT),
			reinterpret_cast<SimulatorInstructionFunction<> *>(instructionFunction));
	}

	template<typename recordT>
	size_t AddRecord()
	{
		return InternalAddRecord(sizeof(recordT), alignof(recordT));
	}

	template<typename instructionT>
	instructionT *CommitInstruction()
	{
		return reinterpret_cast<instructionT *>(InternalCommitInstruction());
	}

	template<typename recordT>
	recordT *GetRecord(size_t offset)
	{
		return reinterpret_cast<recordT *>(InternalGetRecord(offset));
	}

	virtual void BindInputReference(size_t index, types::Boolean const *&inputPointerReference) = 0;
	virtual void BindInputReference(size_t index, types::FixedPointElement const *&inputPointerReference) = 0;

	virtual void BindOutput(size_t index, types::Boolean &outputReference) = 0;
	virtual void BindOutput(size_t index, types::FixedPointElement *outputReference) = 0;
};

} // namespace oddf::simulator::common::backend
