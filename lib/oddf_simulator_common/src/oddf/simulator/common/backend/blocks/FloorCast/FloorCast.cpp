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

#include "../FloorCast.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

FloorCast::FloorCast(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string FloorCast::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

void FloorCast::Elaborate(ISimulatorElaborationContext &context)
{
	auto inputs = GetInputsList();
	auto outputs = GetOutputsList();

	if (inputs->GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (outputs->GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (inputs->Item(0).GetType() == outputs->Item(0).GetType()) {

		// This floor cast is a no-op. Remove it.

		context.TransferConnectivity(outputs->Item(0), inputs->Item(0).GetDriver());
		context.DisconnectInput(inputs->Item(0));
		context.RemoveThisBlock();
	}

	if (inputs->Item(0).GetType().GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::Unsupported);

	if (outputs->Item(0).GetType().GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::Unsupported);
}

/*

// CLEANUP

struct PlusInstruction : public SimulatorInstruction {

    size_t m_resultOffset;
    size_t m_operandCount;

    struct Operand {

        types::FixedPoint const *m_input;
        size_t m_shiftLeft;
        bool m_signed;

    } m_operands[1];

private:

    void Operate(types::FixedPoint &result)
    {
        using element_type = types::FixedPoint::ElementType;
        using intermediate_type = types::FixedPoint::IntermediateType;

        auto *dest = result.m_elements;
        auto dest_length = result.m_length;

        for (size_t dest_i = 0; dest_i < dest_length; ++dest_i)
            dest[dest_i] = 0;

        for (size_t op_i = 0; op_i < m_operandCount; ++op_i) {

            auto const &op = m_operands[op_i];

            auto src_length = op.m_input->m_length;
            auto const *src = op.m_input->m_elements;
            auto src_element_shl = op.m_shiftLeft / types::FixedPoint::ElementBitWidth;
            auto src_bit_shl = op.m_shiftLeft % types::FixedPoint::ElementBitWidth;

            intermediate_type temp = 0;

            size_t dest_i = src_element_shl;

            for (size_t src_i = 0; src_i < src_length && dest_i < dest_length; ++src_i, ++dest_i) {

                temp += static_cast<intermediate_type>(dest[dest_i])
                    + (static_cast<intermediate_type>(src[src_i]) << src_bit_shl);

                dest[dest_i] = static_cast<element_type>(temp);

                temp >>= types::FixedPoint::ElementBitWidth;
            }

            intermediate_type src_extension
                = op.m_signed && (src_length > 0) && (src[src_length - 1] >= types::FixedPoint::SignedMinimumNegativeElement)
                ? types::FixedPoint::SignedExtension
                : 0;

            for (; dest_i < dest_length; ++dest_i) {

                temp += static_cast<intermediate_type>(dest[dest_i])
                    + (src_extension << src_bit_shl);

                dest[dest_i] = static_cast<element_type>(temp);

                temp >>= types::FixedPoint::ElementBitWidth;
            }
        }
    }

public:

    static void
    InstructionFunction(PlusInstruction *instruction)
    {
        types::FixedPoint &result = instruction->GetRecord<types::FixedPoint>(instruction->m_resultOffset);
        instruction->Operate(result);
    }
};

*/

struct FloorCastReduceUnsignedInstruction : public SimulatorInstruction {

	types::FixedPoint::ElementType m_mask;
	types::FixedPoint const *m_input;
	types::FixedPoint m_output;

	static void InstructionFunction(FloorCastReduceUnsignedInstruction *instruction)
	{
		instruction->Operate();
	}

private:

	void Operate()
	{
		for (size_t i = 0; i < m_output.m_length; ++i)
			m_output.m_elements[i] = m_input->m_elements[i];

		m_output.m_elements[m_output.m_length - 1] &= m_mask;
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

void FloorCast::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	EmitFloorCastInstruction(context, GetOutputsList()->Item(0), GetInputsList()->Item(0));
}

} // namespace oddf::simulator::common::backend::blocks
