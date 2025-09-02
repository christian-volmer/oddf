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

#include "../Plus.h"

#include <oddf/Exception.h>

#include <cassert>

namespace oddf::simulator::common::backend::blocks {

Plus::Plus(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string Plus::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

void Plus::Elaborate(ISimulatorElaborationContext &)
{
	auto outputs = GetOutputsList();

	if (outputs->GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (outputs->Item(0).GetType().GetTypeId() != design::NodeType::FIXED_POINT)
		throw Exception(ExceptionCode::Unsupported);

	auto inputs = GetInputsList();

	if (inputs->GetSize() < 1)
		throw Exception(ExceptionCode::Unsupported);

	auto inputsEnumerator = inputs->GetEnumerator();
	while (inputsEnumerator->MoveNext())
		if (inputsEnumerator->GetCurrent().GetType().GetTypeId() != design::NodeType::FIXED_POINT)
			throw Exception(ExceptionCode::Unsupported);
}

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

			/*

			The output type should be wide enough so that overflows cannot occur.
			We should therefore not have to fix the bits above the most significant bit.

			*/
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

void EmitPlusInstruction(ISimulatorCodeGenerationContext &context,
	SimulatorBlockOutput const &output, IListView<SimulatorBlockInput const &> const &inputs)
{
	context.StartInstructionVariadic(
		PlusInstruction::InstructionFunction,
		&PlusInstruction::m_operands,
		inputs.GetSize());

	auto outputType = output.GetType();

	size_t resultOffset = context.AddOutputRecord<types::FixedPoint>(outputType);

	auto *instruction = context.CommitInstruction<PlusInstruction>();

	instruction->m_resultOffset = resultOffset;
	instruction->m_operandCount = inputs.GetSize();

	for (size_t i = 0; i < inputs.GetSize(); ++i) {

		auto inputType = inputs.Item(i).GetType();

		context.BindInputReference(i, instruction->m_operands[i].m_input);

		ptrdiff_t shiftLeft = outputType.GetFraction() - inputType.GetFraction();
		assert(shiftLeft >= 0);

		instruction->m_operands[i].m_shiftLeft = shiftLeft;
		instruction->m_operands[i].m_signed = inputType.IsSigned();
	}

	auto &result = instruction->GetRecord<types::FixedPoint>(resultOffset);
	context.BindOutput(output.GetIndex(), result);
}

void Plus::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	EmitPlusInstruction(context, GetOutputsList()->Item(0), *GetInputsList());
}

} // namespace oddf::simulator::common::backend::blocks
