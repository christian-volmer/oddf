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

#include <oddf/simulator/common/backend/SimulatorBlockOutput.h>
#include <oddf/simulator/common/backend/ISimulatorCodeGenerationContext.h>

namespace oddf::simulator::common::backend::instructions {

template<typename T, typename = void>
class CopyInstruction;

template<typename T>
class CopyInstruction<T, std::void_t<typename T::ValueType>> : public SimulatorInstruction {

private:

	typename T::DataType const *m_source;
	T m_result;

	static void InstructionFunction(CopyInstruction *instruction)
	{
		instruction->m_result.m_value = *instruction->m_source;
	}

public:

	static void Emit(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, typename T::DataType const &source)
	{
		context.StartInstruction<CopyInstruction>(InstructionFunction);
		auto *instruction = context.CommitInstruction<CopyInstruction>();

		instruction->m_source = &source;
		context.BindOutput(output.GetIndex(), instruction->m_result);
	};
};

template<typename T>
class CopyInstruction<T, std::void_t<typename T::ElementType>> : public SimulatorInstruction {

private:

	typename T::ElementType const *m_source;
	T m_result;

	static void InstructionFunction(CopyInstruction *instruction)
	{
		for (size_t i = 0; i < instruction->m_result.m_length; ++i)
			instruction->m_result.m_elements[i] = instruction->m_source[i];
	}

public:

	static void Emit(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, typename T::ElementType const &source)
	{
		auto elementCount = T::GetElementCount(output.GetType());

		context.StartInstructionWithOutput<CopyInstruction>(
			InstructionFunction,
			&CopyInstruction::m_result,
			elementCount);

		auto *instruction = context.CommitInstruction<CopyInstruction>();

		instruction->m_source = &source;
		instruction->m_result.m_length = elementCount;
		context.BindOutput(output.GetIndex(), instruction->m_result);
	};
};

} // namespace oddf::simulator::common::backend::instructions
