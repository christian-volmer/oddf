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

    Implements equality testing with fixed-point operands.

*/

#include "EqualCode.h"

#include <oddf/simulator/common/backend/Types.h>

namespace oddf::simulator::common::backend::blocks {

struct EqualInstructionFixedPoint : public SimulatorInstruction {

	types::Boolean m_result;
	types::FixedPoint const *m_operand0;
	types::FixedPoint const *m_operand1;

	void Operate()
	{
		for (size_t i = 0; i < m_operand0->m_length; ++i)
			if (m_operand0->m_elements[i] != m_operand1->m_elements[i]) {

				m_result.m_value = 0;
				return;
			}

		m_result.m_value = 1;
	}

	static void InstructionFunction(EqualInstructionFixedPoint *instruction)
	{
		instruction->Operate();
	}
};

void EmitEqualCodeFixedPoint(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output,
	IListView<SimulatorBlockInput const &> const &inputs)
{
	if (inputs.GetSize() != 2)
		throw Exception(ExceptionCode::Unexpected);

	if (inputs.Item(0).GetType() != inputs.Item(1).GetType())
		throw Exception(ExceptionCode::Unexpected);

	context.StartInstruction(EqualInstructionFixedPoint::InstructionFunction);
	auto *instruction = context.CommitInstruction<EqualInstructionFixedPoint>();

	context.BindOutput(output.GetIndex(), instruction->m_result);
	context.BindInputReference(inputs.Item(0).GetIndex(), instruction->m_operand0);
	context.BindInputReference(inputs.Item(1).GetIndex(), instruction->m_operand1);
}

template<>
void EmitEqualCode<types::FixedPoint>(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output,
	IListView<SimulatorBlockInput const &> const &inputs)
{
	EmitEqualCodeFixedPoint(context, output, inputs);
}

} // namespace oddf::simulator::common::backend::blocks
