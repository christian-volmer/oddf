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

#include "CopyCode.h"

#include <oddf/simulator/common/backend/Types.h>

namespace oddf::simulator::common::backend::blocks {

template<typename T>
struct CopyInstructionValue : public SimulatorInstruction {

	typename T::DataType const *m_source;
	T m_result;

	static void InstructionFunction(CopyInstructionValue *instruction)
	{
		instruction->m_result.m_value = *instruction->m_source;
	}
};

template<typename T>
void EmitCopyCodeValue(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output,
	typename T::DataType const *source)
{
	context.StartInstruction(CopyInstructionValue<T>::InstructionFunction);
	auto *instruction = context.CommitInstruction<CopyInstructionValue<T>>();

	instruction->m_source = source;
	context.BindOutput(output.GetIndex(), instruction->m_result);
}

template<>
void EmitCopyCode<types::Boolean>(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output,
	types::Boolean::DataType const *source)
{
	EmitCopyCodeValue<types::Boolean>(context, output, source);
}

} // namespace oddf::simulator::common::backend::blocks
