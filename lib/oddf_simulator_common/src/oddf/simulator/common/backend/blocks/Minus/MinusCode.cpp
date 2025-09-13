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

#include "MinusCode.h"

#include <oddf/simulator/common/backend/Types.h>

namespace oddf::simulator::common::backend::blocks {

struct MinusInstructionBase : public SimulatorInstruction {

	types::FixedPoint const *m_operand;
	types::FixedPoint m_result;
};

struct MinusInstruction : public MinusInstructionBase {

	void Operate()
	{
		size_t i = 0;
		for (i = 0; i < m_operand->m_length && m_operand->m_elements[i] == 0; ++i)
			m_result.m_elements[i] = 0;

		if (i < m_operand->m_length) {

			m_result.m_elements[i] = ~m_operand->m_elements[i] + 1;
			++i;

			for (; i < m_operand->m_length; ++i)
				m_result.m_elements[i] = ~m_operand->m_elements[i];
		}
	}

	static void InstructionFunction(MinusInstruction *instruction)
	{
		instruction->Operate();
	}
};

struct MinusInstructionExpandUnsigned : public MinusInstructionBase {

	void Operate()
	{
		size_t i = 0;
		for (i = 0; i < m_operand->m_length && m_operand->m_elements[i] == 0; ++i)
			m_result.m_elements[i] = 0;

		if (i < m_operand->m_length) {

			m_result.m_elements[i] = ~m_operand->m_elements[i] + 1;
			++i;

			for (; i < m_operand->m_length; ++i)
				m_result.m_elements[i] = ~m_operand->m_elements[i];

			m_result.m_elements[i] = types::FixedPoint::SignedExtension;
		}
		else
			m_result.m_elements[i] = 0;
	}

	static void InstructionFunction(MinusInstructionExpandUnsigned *instruction)
	{
		instruction->Operate();
	}
};

struct MinusInstructionExpandSigned : public MinusInstructionBase {

	void Operate()
	{
		size_t i = 0;
		for (i = 0; i < m_operand->m_length && m_operand->m_elements[i] == 0; ++i)
			m_result.m_elements[i] = 0;

		if (i < m_operand->m_length) {

			m_result.m_elements[i] = ~m_operand->m_elements[i] + 1;
			++i;

			for (; i < m_operand->m_length; ++i)
				m_result.m_elements[i] = ~m_operand->m_elements[i];

			if (m_operand->m_elements[i - 1] >= types::FixedPoint::SignedMinimumNegativeElement)
				m_result.m_elements[i] = 0;
			else
				m_result.m_elements[i] = types::FixedPoint::SignedExtension;
		}
		else
			m_result.m_elements[i] = 0;
	}

	static void InstructionFunction(MinusInstructionExpandSigned *instruction)
	{
		instruction->Operate();
	}
};

MinusInstructionBase *CreateMinusInstruction(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output,
	SimulatorBlockInput const &input)
{
	auto inputType = input.GetType();

	auto inputElementCount = types::FixedPoint::GetElementCount(inputType);
	auto outputElementCount = types::FixedPoint::GetElementCount(output.GetType());

	if (outputElementCount == inputElementCount) {

		context.StartInstructionWithOutput(
			MinusInstruction::InstructionFunction,
			&MinusInstruction::m_result,
			outputElementCount);

		return context.CommitInstruction<MinusInstruction>();
	}
	else if (outputElementCount == inputElementCount + 1) {

		if (inputType.IsUnsigned()) {

			context.StartInstructionWithOutput(
				MinusInstructionExpandUnsigned::InstructionFunction,
				&MinusInstructionExpandUnsigned::m_result,
				outputElementCount);

			return context.CommitInstruction<MinusInstructionExpandUnsigned>();
		}
		else {

			context.StartInstructionWithOutput(
				MinusInstructionExpandSigned::InstructionFunction,
				&MinusInstructionExpandSigned::m_result,
				outputElementCount);

			return context.CommitInstruction<MinusInstructionExpandSigned>();
		}
	}
	else
		throw Exception(ExceptionCode::Unexpected);
}

void EmitMinusCode(ISimulatorCodeGenerationContext &context, IListView<SimulatorBlockOutput const &> const &outputs,
	IListView<SimulatorBlockInput const &> const &inputs)
{
	size_t outputsCount = outputs.GetSize();

	for (size_t i = 0; i < outputsCount; ++i) {

		auto const &output = outputs.Item(i);
		auto const &input = inputs.Item(i);

		auto *instruction = CreateMinusInstruction(context, output, input);

		context.BindOutput(output.GetIndex(), instruction->m_result);
		context.BindInputReference(input.GetIndex(), instruction->m_operand);
	}
}

} // namespace oddf::simulator::common::backend::blocks
