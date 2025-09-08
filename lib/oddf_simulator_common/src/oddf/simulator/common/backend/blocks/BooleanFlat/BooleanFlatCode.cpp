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

    Simulator support for the Boolean operators AND, OR, and XOR.

*/

#include "BooleanFlatCode.h"

#include <oddf/simulator/common/backend/Types.h>

#include <functional>

namespace oddf::simulator::common::backend::blocks {

enum class OperationName {

	UNDEFINED = 0,
	AND = 1,
	OR = 2,
	XOR = 3
};

struct BooleanFlatInstructionBase : public SimulatorInstruction {

	types::Boolean m_result;
	size_t m_operandCount;
	types::Boolean const *m_operands[1];
};

template<typename functionT, bool identityValue>
struct BooleanFlatInstruction : public BooleanFlatInstructionBase {

	using BooleanFlatInstructionBase::m_operands;

	void Operate()
	{
		bool result = identityValue;
		for (size_t i = 0; i < m_operandCount; ++i)
			result = functionT {}(result, m_operands[i]->m_value != 0);

		m_result.m_value = result;
	}

	static void InstructionFunction(BooleanFlatInstruction *instruction)
	{
		instruction->Operate();
	}
};

using BooleanAndInstruction = BooleanFlatInstruction<std::logical_and<bool>, true>;
using BooleanOrInstruction = BooleanFlatInstruction<std::logical_or<bool>, false>;
using BooleanXorInstruction = BooleanFlatInstruction<std::not_equal_to<bool>, false>;

template<typename T>
BooleanFlatInstructionBase *CreateBooleanFlatInstruction(ISimulatorCodeGenerationContext &context, size_t inputsCount)
{
	// THe static_cast should not be necessary. Bug in GCC?
	using variadicMemberT = types::Boolean const *(T::*)[1];

	context.StartInstructionVariadic<T>(T::InstructionFunction, static_cast<variadicMemberT>(&T::m_operands), inputsCount);
	return context.CommitInstruction<T>();
}

void EmitBooleanFlatCode(OperationName operationName, ISimulatorCodeGenerationContext &context, size_t inputsCount,
	size_t outputsCount)
{
	size_t inputsPerInstruction = inputsCount / outputsCount;

	size_t inputsStartIndex = 0;
	for (size_t outputIndex = 0; outputIndex < outputsCount; ++outputIndex) {

		BooleanFlatInstructionBase *instruction;

		switch (operationName) {

			case OperationName::AND:
				instruction = CreateBooleanFlatInstruction<BooleanAndInstruction>(context, inputsPerInstruction);
				break;

			case OperationName::OR:
				instruction = CreateBooleanFlatInstruction<BooleanOrInstruction>(context, inputsPerInstruction);
				break;

			case OperationName::XOR:
				instruction = CreateBooleanFlatInstruction<BooleanXorInstruction>(context, inputsPerInstruction);
				break;

			default:
				throw Exception(ExceptionCode::Unexpected);
		}

		instruction->m_operandCount = inputsPerInstruction;

		for (size_t i = 0; i < inputsPerInstruction; ++i)
			context.BindInputReference(inputsStartIndex + i, instruction->m_operands[i]);

		context.BindOutput(outputIndex, instruction->m_result);

		inputsStartIndex += inputsPerInstruction;
	}
}

void EmitBooleanAndCode(ISimulatorCodeGenerationContext &context, size_t inputsCount, size_t outputsCount)
{
	EmitBooleanFlatCode(OperationName::AND, context, inputsCount, outputsCount);
}

void EmitBooleanOrCode(ISimulatorCodeGenerationContext &context, size_t inputsCount, size_t outputsCount)
{
	EmitBooleanFlatCode(OperationName::OR, context, inputsCount, outputsCount);
}

void EmitBooleanXorCode(ISimulatorCodeGenerationContext &context, size_t inputsCount, size_t outputsCount)
{
	EmitBooleanFlatCode(OperationName::XOR, context, inputsCount, outputsCount);
}

} // namespace oddf::simulator::common::backend::blocks
