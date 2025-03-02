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

#include <oddf/simulator/common/backend/Types.h>
#include <oddf/simulator/common/backend/SimulatorInstructionBase.h>

namespace oddf::simulator::common::backend::instructions {

template<typename T, typename = void>
struct Copy;

template<typename T>
struct Copy<T, std::void_t<typename T::ValueType>> : public SimulatorInstructionBase {

private:

	T const *m_pSource;
	T m_output;

	static size_t InstructionFunction(Copy &instruction)
	{
		instruction.m_output.m_value = instruction.m_pSource->m_value;
		return sizeof(instruction);
	}

public:

	Copy(ISimulatorCodeGenerationContext &context, T const &source) :
		SimulatorInstructionBase(&InstructionFunction),
		m_pSource(&source),
		m_output()
	{
		context.RegisterOutput(0, m_output);
	}
};

template<typename T>
struct Copy<T, std::void_t<typename T::ElementType>> : public SimulatorInstructionBase {

private:

	size_t m_byteCount;
	T const *m_pSource;
	T m_output[1];

	static size_t InstructionFunction(Copy &instruction)
	{
		memcpy(static_cast<void *>(&instruction.m_output[0]), instruction.m_pSource, instruction.m_byteCount);
		return sizeof(instruction) - sizeof(T) + instruction.m_byteCount;
	}

public:

	static auto GetVariadicMember()
	{
		return &Copy::m_output;
	}

	Copy(ISimulatorCodeGenerationContext &context, T const &source, size_t elementCount) :
		SimulatorInstructionBase(&InstructionFunction),
		m_byteCount(elementCount * sizeof(T)),
		m_pSource(&source),
		m_output()
	{
		context.RegisterOutput(0, m_output, elementCount);
	}
};

} // namespace oddf::simulator::common::backend::instructions
