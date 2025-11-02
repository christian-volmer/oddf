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

    Implements the floor-cast operation between fixed-point numbers.

*/

#include "FloorCastCode.h"

#include <oddf/simulator/common/backend/Types.h>

namespace oddf::simulator::common::backend::blocks {

struct FloorCastShiftRightInstruction : public SimulatorInstruction {

	size_t m_inputElementShr;
	size_t m_inputBitShr;
	bool m_inputSigned;
	types::FixedPoint const *m_input;

	types::FixedPoint::ElementType m_outputPartialMask;
	bool m_outputSigned;
	types::FixedPoint m_output;

	void Operate()
	{
		using types::FixedPoint;
		using element_type = FixedPoint::ElementType;
		using intermediate_type = FixedPoint::IntermediateType;

		auto const *input = m_input->m_elements;
		auto inputLength = m_input->m_length;

		auto *output = m_output.m_elements;
		auto outputLength = m_output.m_length;

		bool inputNegative = m_inputSigned && (input[inputLength - 1] >= FixedPoint::SignedMinimumNegativeElement);
		element_type inputExtension = inputNegative ? FixedPoint::SignedExtension : 0;

		size_t output_i = 0;

		if (m_inputElementShr < inputLength) {

			size_t input_i = m_inputElementShr;
			intermediate_type temp = input[input_i] >> m_inputBitShr;

			for (; output_i < outputLength && input_i + 1 < inputLength; ++output_i, ++input_i) {

				temp += static_cast<intermediate_type>(input[input_i + 1]) << (FixedPoint::ElementBitSize - m_inputBitShr);
				output[output_i] = static_cast<element_type>(temp);
				temp >>= FixedPoint::ElementBitSize;
			}

			if (output_i < outputLength && input_i < inputLength) {

				temp += static_cast<intermediate_type>(inputExtension) << (FixedPoint::ElementBitSize - m_inputBitShr);
				output[output_i] = static_cast<element_type>(temp);
				++output_i;
			}
		}

		if (output_i < outputLength) {

			for (; output_i < outputLength; ++output_i)
				output[output_i] = inputExtension;
		}
		else {

			element_type outputSignBit = m_outputPartialMask - (m_outputPartialMask >> 1);
			bool outputNegative = m_outputSigned && (output[outputLength - 1] & outputSignBit);

			if (outputNegative)
				output[outputLength - 1] |= ~m_outputPartialMask;
			else
				output[outputLength - 1] &= m_outputPartialMask;
		}
	}

	static void InstructionFunction(FloorCastShiftRightInstruction *instruction)
	{
		instruction->Operate();
	}
};

struct FloorCastShiftLeftInstruction : public SimulatorInstruction {

	size_t m_inputElementShl;
	size_t m_inputBitShl;
	bool m_inputSigned;
	types::FixedPoint const *m_input;

	types::FixedPoint::ElementType m_outputPartialMask;
	bool m_outputSigned;
	types::FixedPoint m_output;

	void Operate()
	{
		using types::FixedPoint;
		using element_type = FixedPoint::ElementType;
		using intermediate_type = FixedPoint::IntermediateType;

		auto const *input = m_input->m_elements;
		auto inputLength = m_input->m_length;

		auto *output = m_output.m_elements;
		auto outputLength = m_output.m_length;

		size_t output_i = 0;
		for (; output_i < outputLength && output_i < m_inputElementShl; ++output_i)
			output[output_i] = 0;

		intermediate_type temp = 0;

		size_t input_i = 0;
		for (; output_i < outputLength && input_i < inputLength; ++output_i, ++input_i) {

			temp += static_cast<intermediate_type>(input[input_i]) << m_inputBitShl;

			output[output_i] = static_cast<element_type>(temp);

			temp >>= FixedPoint::ElementBitSize;
		}

		bool inputNegative = m_inputSigned && (input[inputLength - 1] >= FixedPoint::SignedMinimumNegativeElement);
		intermediate_type inputExtension = inputNegative ? FixedPoint::SignedExtension : 0;

		for (; output_i < outputLength; ++output_i) {

			temp += inputExtension << m_inputBitShl;

			output[output_i] = static_cast<element_type>(temp);

			temp >>= FixedPoint::ElementBitSize;
		}

		element_type outputSignBit = m_outputPartialMask - (m_outputPartialMask >> 1);
		bool outputNegative = m_outputSigned && (output[outputLength - 1] & outputSignBit);

		if (outputNegative)
			output[outputLength - 1] |= ~m_outputPartialMask;
		else
			output[outputLength - 1] &= m_outputPartialMask;
	}

	static void InstructionFunction(FloorCastShiftLeftInstruction *instruction)
	{
		instruction->Operate();
	}
};

void EmitFloorCastInstruction(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, SimulatorBlockInput const &input)
{
	auto inputType = input.GetType();
	auto outputType = output.GetType();

	auto outputElementCount = types::FixedPoint::GetElementCount(outputType);

	ptrdiff_t shiftLeft = outputType.GetFraction() - inputType.GetFraction();

	if (shiftLeft >= 0) {

		context.StartInstructionWithOutput(
			FloorCastShiftLeftInstruction::InstructionFunction,
			&FloorCastShiftLeftInstruction::m_output,
			outputElementCount);

		auto *instruction = context.CommitInstruction<FloorCastShiftLeftInstruction>();

		instruction->m_inputElementShl = static_cast<size_t>(shiftLeft) / types::FixedPoint::ElementBitSize;
		instruction->m_inputBitShl = static_cast<size_t>(shiftLeft) % types::FixedPoint::ElementBitSize;
		instruction->m_inputSigned = inputType.IsSigned();

		// The number of valid bits in the most significant element, or 0 if all bits
		// are valid.
		size_t outputPartialElementBitCount = outputType.GetWordWidth() % types::FixedPoint::ElementBitSize;

		// Mask of the valid bits (that are not part of the extension/padding) of the
		// most significant element.
		instruction->m_outputPartialMask = outputPartialElementBitCount == 0
			? types::FixedPoint::SignedExtension
			: ~(types::FixedPoint::SignedExtension << outputPartialElementBitCount);

		instruction->m_outputSigned = outputType.IsSigned();

		context.BindInputReference(input.GetIndex(), instruction->m_input);
		context.BindOutput(output.GetIndex(), instruction->m_output);
	}
	else {

		auto shiftRight = -shiftLeft;

		context.StartInstructionWithOutput(
			FloorCastShiftRightInstruction::InstructionFunction,
			&FloorCastShiftRightInstruction::m_output,
			outputElementCount);

		auto *instruction = context.CommitInstruction<FloorCastShiftRightInstruction>();

		instruction->m_inputElementShr = static_cast<size_t>(shiftRight) / types::FixedPoint::ElementBitSize;
		instruction->m_inputBitShr = static_cast<size_t>(shiftRight) % types::FixedPoint::ElementBitSize;
		instruction->m_inputSigned = inputType.IsSigned();

		// The number of valid bits in the most significant element, or 0 if all bits
		// are valid.
		size_t outputPartialElementBitCount = outputType.GetWordWidth() % types::FixedPoint::ElementBitSize;

		// Mask of the valid bits (that are not part of the extension/padding) of the
		// most significant element.
		instruction->m_outputPartialMask = outputPartialElementBitCount == 0
			? types::FixedPoint::SignedExtension
			: ~(types::FixedPoint::SignedExtension << outputPartialElementBitCount);

		instruction->m_outputSigned = outputType.IsSigned();

		context.BindInputReference(input.GetIndex(), instruction->m_input);
		context.BindOutput(output.GetIndex(), instruction->m_output);
	}
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
