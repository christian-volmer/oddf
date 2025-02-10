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

#include "SimulatorInstructionBase.h"

#include "Types.h"

#include "ISimulatorComponentContext.h"

#include <type_traits>
#include <cstdint>

namespace oddf::simulator::common::backend {

class ISimulatorCodeGenerationContext : public virtual ISimulatorComponentContext {

protected:

	virtual void *InternalEmitInstruction(size_t size, size_t alignment) = 0;

public:

	virtual ~ISimulatorCodeGenerationContext() = default;

	template<typename classT, typename... argTs>
	void EmitInstruction(argTs &&...args)
	{
		auto *instruction = InternalEmitInstruction(sizeof(classT), alignof(classT));
		new (instruction) classT(*this, std::forward<argTs>(args)...);
	}

	template<typename classT, typename memberT, typename... argTs>
	void EmitInstructionVariadic(memberT (classT::*variaticMember)[1], size_t count, argTs &&...args)
	{
		alignas(classT) char tempStorage[sizeof(classT)];
		size_t offset = reinterpret_cast<char const *>(&(reinterpret_cast<classT const *>(&tempStorage)->*variaticMember)) - tempStorage;

		// TODO: hier sollten wir checken, dass der variadic member auch der letzte member ist...

		size_t size = offset + count * sizeof(memberT);

		// Round up to the next multiple of alignof(classT)
		size = ((size + alignof(classT) - 1) / alignof(classT)) * alignof(classT);

		auto *instruction = InternalEmitInstruction(size, alignof(classT));
		new (instruction) classT(*this, std::forward<argTs>(args)...);
	}

	virtual void RegisterInput(size_t index, types::Boolean const *&inputPointerReference) = 0;

	virtual void RegisterOutput(size_t index, types::Boolean &outputReference) = 0;
	virtual void RegisterOutput(size_t index, types::FixedPointElement *outputReference, size_t elementCount) = 0;
};

} // namespace oddf::simulator::common::backend
