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

#include "../Not.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

NotMaster::NotMaster(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string NotMaster::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath();
}

void NotMaster::Elaborate(ISimulatorElaborationContext &)
{
	auto outputs = GetOutputsList();

	if (outputs.GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (outputs[0].GetType().GetTypeId() != design::NodeType::BOOLEAN)
		throw Exception(ExceptionCode::Unsupported);

	auto inputs = GetInputsList();

	if (inputs.GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (inputs[0].GetType().GetTypeId() != design::NodeType::BOOLEAN)
		throw Exception(ExceptionCode::Unsupported);
}

// CLEANUP: where do we put instructions?

class NotInstruction : public SimulatorInstruction {

private:

	types::Boolean const *m_operand;
	types::Boolean m_result;

	static void InstructionFunction(NotInstruction *instruction)
	{
		instruction->m_result.m_value = (~(instruction->m_operand->m_value)) & 1;
	}

public:

	static void Emit(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, SimulatorBlockInput const &input)
	{
		context.StartInstruction(InstructionFunction);
		auto *instruction = context.CommitInstruction<NotInstruction>();

		context.BindInputReference(input.GetIndex(), instruction->m_operand);
		context.BindOutput(output.GetIndex(), instruction->m_result);
	}
};

void NotMaster::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	NotInstruction::Emit(context, GetOutputsList()[0], GetInputsList()[0]);
}

} // namespace oddf::simulator::common::backend::blocks
