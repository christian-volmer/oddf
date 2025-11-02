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

    Implements equality testing with Boolean operands.

*/

#include "EqualCode.h"

#include <oddf/simulator/common/backend/Types.h>

namespace oddf::simulator::common::backend::blocks {

struct EqualInstructionBoolean : public SimulatorInstruction {

	types::Boolean m_result;
	types::Boolean const *m_operand0;
	types::Boolean const *m_operand1;

	void Operate()
	{
		m_result.m_value = (m_operand0->m_value == 0) == (m_operand1->m_value == 0);
	}

	static void InstructionFunction(EqualInstructionBoolean *instruction)
	{
		instruction->Operate();
	}
};

void EmitEqualCodeBoolean(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output,
	IListView<SimulatorBlockInput const &> const &inputs)
{
	if (inputs.GetSize() != 2)
		throw Exception(ExceptionCode::Unexpected);

	context.StartInstruction(EqualInstructionBoolean::InstructionFunction);
	auto *instruction = context.CommitInstruction<EqualInstructionBoolean>();

	context.BindOutput(output.GetIndex(), instruction->m_result);
	context.BindInputReference(inputs.Item(0).GetIndex(), instruction->m_operand0);
	context.BindInputReference(inputs.Item(1).GetIndex(), instruction->m_operand1);
}

template<>
void EmitEqualCode<types::Boolean>(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output,
	IListView<SimulatorBlockInput const &> const &inputs)
{
	EmitEqualCodeBoolean(context, output, inputs);
}

} // namespace oddf::simulator::common::backend::blocks
