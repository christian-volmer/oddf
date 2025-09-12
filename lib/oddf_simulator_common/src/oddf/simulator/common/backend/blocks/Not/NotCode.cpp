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

#include "NotCode.h"

#include <oddf/simulator/common/backend/Types.h>

namespace oddf::simulator::common::backend::blocks {

struct NotInstruction : public SimulatorInstruction {

	types::Boolean m_result;
	types::Boolean const *m_operand;

	void Operate()
	{
		m_result.m_value = (m_operand->m_value == 0);
	}

	static void InstructionFunction(NotInstruction *instruction)
	{
		instruction->Operate();
	}
};

void EmitNotCode(ISimulatorCodeGenerationContext &context, IListView<SimulatorBlockOutput const &> const &outputs,
	IListView<SimulatorBlockInput const &> const &inputs)
{
	size_t outputsCount = outputs.GetSize();

	for (size_t i = 0; i < outputsCount; ++i) {

		auto const &output = outputs.Item(i);
		auto const &input = inputs.Item(i);

		context.StartInstruction(NotInstruction::InstructionFunction);
		auto *instruction = context.CommitInstruction<NotInstruction>();

		context.BindOutput(output.GetIndex(), instruction->m_result);
		context.BindInputReference(input.GetIndex(), instruction->m_operand);
	}
}

} // namespace oddf::simulator::common::backend::blocks
