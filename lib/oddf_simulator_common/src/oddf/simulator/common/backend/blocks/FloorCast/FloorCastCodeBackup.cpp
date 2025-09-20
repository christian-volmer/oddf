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

#include "FloorCastCode.h"

#include <oddf/simulator/common/backend/Types.h>

namespace oddf::simulator::common::backend::blocks {

struct FloorCastReduceUnsignedInstruction : public SimulatorInstruction {

	types::FixedPoint::ElementType m_mask;
	types::FixedPoint const *m_input;
	types::FixedPoint m_output;

	void Operate()
	{
		for (size_t i = 0; i < m_output.m_length; ++i)
			m_output.m_elements[i] = m_input->m_elements[i];

		m_output.m_elements[m_output.m_length - 1] &= m_mask;
	}

	static void InstructionFunction(FloorCastReduceUnsignedInstruction *instruction)
	{
		instruction->Operate();
	}
};

void EmitFloorCastInstruction(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, SimulatorBlockInput const &input)
{
	auto inputType = input.GetType();
	auto outputType = output.GetType();

	auto inputElementCount = types::FixedPoint::GetElementCount(inputType);
	auto outputElementCount = types::FixedPoint::GetElementCount(outputType);

	ptrdiff_t shiftLeft = outputType.GetFraction() - inputType.GetFraction();

	// Shifting of content still has to be implemented.
	if (shiftLeft != 0)
		throw Exception(ExceptionCode::NotImplemented);

	// Need special treatment if the output has zero width.
	if (outputElementCount < 1)
		throw Exception(ExceptionCode::NotImplemented);

	if (outputElementCount <= inputElementCount) {

		if (outputType.IsUnsigned()) {

			context.StartInstructionWithOutput(
				FloorCastReduceUnsignedInstruction::InstructionFunction,
				&FloorCastReduceUnsignedInstruction::m_output,
				outputElementCount);

			auto *instruction = context.CommitInstruction<FloorCastReduceUnsignedInstruction>();

			instruction->m_mask = types::FixedPoint::SignedExtension
				>> ((types::FixedPoint::ElementBitSize - (outputType.GetWordWidth() % types::FixedPoint::ElementBitSize)) % types::FixedPoint::ElementBitSize);

			/*

			8 --> 0b11111111
			7 --> 0x01111111
			6 --> 0x00111111

			*/

			context.BindInputReference(input.GetIndex(), instruction->m_input);
			context.BindOutput(output.GetIndex(), instruction->m_output);

			return;
		}
	}

	throw Exception(ExceptionCode::Unexpected);
}

void EmitFloorCastCode(ISimulatorCodeGenerationContext &context, IListView<SimulatorBlockOutput const &> const &outputs,
	IListView<SimulatorBlockInput const &> const &inputs)
{
	size_t outputsCount = outputs.GetSize();

	for (size_t i = 0; i < outputsCount; ++i) {

		auto const &output = outputs.Item(i);
		auto const &input = inputs.Item(i);

		EmitFloorCastInstruction(context, output, input);
	}
}

} // namespace oddf::simulator::common::backend::blocks
