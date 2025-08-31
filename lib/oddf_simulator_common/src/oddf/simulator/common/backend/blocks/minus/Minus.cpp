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

#include "../Minus.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

Minus::Minus(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string Minus::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

void Minus::Elaborate(ISimulatorElaborationContext &)
{
	auto outputs = GetOutputsList();

	if (outputs->GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (outputs->Item(0).GetType().GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::Unsupported);

	auto inputs = GetInputsList();

	if (inputs->GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (inputs->Item(0).GetType().GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::Unsupported);
}

// CLEANUP

struct MinusInstruction : public SimulatorInstruction {

	types::FixedPoint const *m_input;
	types::FixedPoint m_output;

	static void InstructionFunction(MinusInstruction *instruction)
	{
		instruction->Operate();
	}

private:

	void Operate()
	{
		size_t i = 0;
		for (i = 0; i < m_input->m_length && m_input->m_elements[i] == 0; ++i)
			m_output.m_elements[i] = 0;

		if (i < m_input->m_length) {

			m_output.m_elements[i] = ~m_input->m_elements[i] + 1;
			++i;

			for (; i < m_input->m_length; ++i)
				m_output.m_elements[i] = ~m_input->m_elements[i];
		}
	}
};

struct MinusInstructionExpandUnsigned : public SimulatorInstruction {

	types::FixedPoint const *m_input;
	types::FixedPoint m_output;

	static void InstructionFunction(MinusInstructionExpandUnsigned *instruction)
	{
		instruction->Operate();
	}

private:

	void Operate()
	{
		size_t i = 0;
		for (i = 0; i < m_input->m_length && m_input->m_elements[i] == 0; ++i)
			m_output.m_elements[i] = 0;

		if (i < m_input->m_length) {

			m_output.m_elements[i] = ~m_input->m_elements[i] + 1;
			++i;

			for (; i < m_input->m_length; ++i)
				m_output.m_elements[i] = ~m_input->m_elements[i];

			m_output.m_elements[i] = types::FixedPoint::SignedExtension;
		}
		else
			m_output.m_elements[i] = 0;
	}
};

struct MinusInstructionExpandSigned : public SimulatorInstruction {

	types::FixedPoint const *m_input;
	types::FixedPoint m_output;

	static void InstructionFunction(MinusInstructionExpandSigned *instruction)
	{
		instruction->Operate();
	}

private:

	void Operate()
	{
		size_t i = 0;
		for (i = 0; i < m_input->m_length && m_input->m_elements[i] == 0; ++i)
			m_output.m_elements[i] = 0;

		if (i < m_input->m_length) {

			m_output.m_elements[i] = ~m_input->m_elements[i] + 1;
			++i;

			for (; i < m_input->m_length; ++i)
				m_output.m_elements[i] = ~m_input->m_elements[i];

			if (m_input->m_elements[i - 1] >= types::FixedPoint::SignedMinimumNegativeElement)
				m_output.m_elements[i] = 0;
			else
				m_output.m_elements[i] = types::FixedPoint::SignedExtension;
		}
		else
			m_output.m_elements[i] = 0;
	}
};

void EmitMinusInstruction(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, SimulatorBlockInput const &input)
{
	auto inputType = input.GetType();

	auto inputElementCount = types::FixedPoint::RequiredElementCount(inputType);
	auto outputElementCount = types::FixedPoint::RequiredElementCount(output.GetType());

	if (outputElementCount == inputElementCount) {

		context.StartInstructionWithOutput(
			MinusInstruction::InstructionFunction,
			&MinusInstruction::m_output,
			outputElementCount);

		auto *instruction = context.CommitInstruction<MinusInstruction>();

		context.BindInputReference(input.GetIndex(), instruction->m_input);
		context.BindOutput(output.GetIndex(), instruction->m_output);
	}
	else if (outputElementCount == inputElementCount + 1) {

		if (inputType.IsUnsigned()) {

			context.StartInstructionWithOutput(
				MinusInstructionExpandUnsigned::InstructionFunction,
				&MinusInstructionExpandUnsigned::m_output,
				outputElementCount);

			auto *instruction = context.CommitInstruction<MinusInstructionExpandUnsigned>();

			context.BindInputReference(input.GetIndex(), instruction->m_input);
			context.BindOutput(output.GetIndex(), instruction->m_output);
		}
		else {

			context.StartInstructionWithOutput(
				MinusInstructionExpandSigned::InstructionFunction,
				&MinusInstructionExpandSigned::m_output,
				outputElementCount);

			auto *instruction = context.CommitInstruction<MinusInstructionExpandSigned>();

			context.BindInputReference(input.GetIndex(), instruction->m_input);
			context.BindOutput(output.GetIndex(), instruction->m_output);
		}
	}
	else
		throw Exception(ExceptionCode::Unexpected);
}

void Minus::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	EmitMinusInstruction(context, GetOutputsList()->Item(0), GetInputsList()->Item(0));
}

} // namespace oddf::simulator::common::backend::blocks
