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
	return GetDesignBlockReference()->GetPath();
}

void Minus::Elaborate(ISimulatorElaborationContext &)
{
	auto outputs = GetOutputsList();

	if (outputs.GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (outputs[0].GetType().GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::Unsupported);

	auto inputs = GetInputsList();

	if (inputs.GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (inputs[0].GetType().GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::Unsupported);
}

// CLEANUP

struct MinusInstruction : public SimulatorInstruction {

	size_t m_elementCount;
	types::FixedPointElement const *m_input;
	types::FixedPointElement m_output[1];

	static void InstructionFunction(MinusInstruction *instruction)
	{
		instruction->Operate();
	}

private:

	void Operate()
	{
		size_t i = 0;
		for (i = 0; i < m_elementCount && m_input[i].m_content == 0; ++i)
			m_output[i].m_content = 0;

		if (i < m_elementCount) {

			m_output[i].m_content = ~m_input[i].m_content + 1;
			++i;

			for (; i < m_elementCount; ++i)
				m_output[i].m_content = ~m_input[i].m_content;
		}
	}
};

struct MinusInstructionExpandUnsigned : public SimulatorInstruction {

	size_t m_inputElementCount;
	types::FixedPointElement const *m_input;
	types::FixedPointElement m_output[1];

	static void InstructionFunction(MinusInstructionExpandUnsigned *instruction)
	{
		instruction->Operate();
	}

private:

	void Operate()
	{
		size_t i = 0;
		for (i = 0; i < m_inputElementCount && m_input[i].m_content == 0; ++i)
			m_output[i].m_content = 0;

		if (i < m_inputElementCount) {

			m_output[i].m_content = ~m_input[i].m_content + 1;
			++i;

			for (; i < m_inputElementCount; ++i)
				m_output[i].m_content = ~m_input[i].m_content;

			m_output[i].m_content = types::FixedPointElement::SignedExtension;
		}
		else
			m_output[i].m_content = 0;
	}
};

struct MinusInstructionExpandSigned : public SimulatorInstruction {

	size_t m_inputElementCount;
	types::FixedPointElement const *m_input;
	types::FixedPointElement m_output[1];

	static void InstructionFunction(MinusInstructionExpandSigned *instruction)
	{
		instruction->Operate();
	}

private:

	void Operate()
	{
		size_t i = 0;
		for (i = 0; i < m_inputElementCount && m_input[i].m_content == 0; ++i)
			m_output[i].m_content = 0;

		if (i < m_inputElementCount) {

			m_output[i].m_content = ~m_input[i].m_content + 1;
			++i;

			for (; i < m_inputElementCount; ++i)
				m_output[i].m_content = ~m_input[i].m_content;

			if (m_input[i - 1].m_content >= types::FixedPointElement::SignedMinimumNegativeElement)
				m_output[i].m_content = 0;
			else
				m_output[i].m_content = types::FixedPointElement::SignedExtension;
		}
		else
			m_output[i].m_content = 0;
	}
};

void EmitMinusInstruction(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, SimulatorBlockInput const &input)
{
	auto inputType = input.GetType();

	auto inputElementCount = types::FixedPointElement::RequiredElementCount(inputType);
	auto outputElementCount = types::FixedPointElement::RequiredElementCount(output.GetType());

	if (outputElementCount == inputElementCount) {

		context.StartInstructionVariadic(
			MinusInstruction::InstructionFunction,
			&MinusInstruction::m_output,
			outputElementCount);

		auto *instruction = context.CommitInstruction<MinusInstruction>();

		instruction->m_elementCount = inputElementCount;

		context.BindInputReference(input.GetIndex(), instruction->m_input);
		context.BindOutput(output.GetIndex(), instruction->m_output);
	}
	else if (outputElementCount == inputElementCount + 1) {

		if (inputType.IsUnsigned()) {

			context.StartInstructionVariadic(
				MinusInstructionExpandUnsigned::InstructionFunction,
				&MinusInstructionExpandUnsigned::m_output,
				outputElementCount);

			auto *instruction = context.CommitInstruction<MinusInstructionExpandUnsigned>();

			instruction->m_inputElementCount = inputElementCount;

			context.BindInputReference(input.GetIndex(), instruction->m_input);
			context.BindOutput(output.GetIndex(), instruction->m_output);
		}
		else {

			context.StartInstructionVariadic(
				MinusInstructionExpandSigned::InstructionFunction,
				&MinusInstructionExpandSigned::m_output,
				outputElementCount);

			auto *instruction = context.CommitInstruction<MinusInstructionExpandSigned>();

			instruction->m_inputElementCount = inputElementCount;

			context.BindInputReference(input.GetIndex(), instruction->m_input);
			context.BindOutput(output.GetIndex(), instruction->m_output);
		}
	}
	else
		throw Exception(ExceptionCode::Unexpected);
}

void Minus::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	EmitMinusInstruction(context, GetOutputsList()[0], GetInputsList()[0]);
}

} // namespace oddf::simulator::common::backend::blocks
