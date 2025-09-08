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
struct CopyInstructionElement : public SimulatorInstruction {

	typename T::ElementType const *m_source;
	T m_result;

	static void InstructionFunction(CopyInstructionElement *instruction)
	{
		for (size_t i = 0; i < instruction->m_result.m_length; ++i)
			instruction->m_result.m_elements[i] = instruction->m_source[i];
	}
};

template<typename T>
void EmitCopyCodeValue(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output,
	typename T::DataType const *source)
{
	auto elementCount = T::GetElementCount(output.GetType());

	context.StartInstructionWithOutput(CopyInstructionElement<T>::InstructionFunction, &CopyInstructionElement<T>::m_result,
		elementCount);

	auto *instruction = context.CommitInstruction<CopyInstructionElement<T>>();

	instruction->m_source = source;
	instruction->m_result.m_length = elementCount;
	context.BindOutput(output.GetIndex(), instruction->m_result);
}

template<>
void EmitCopyCode<types::FixedPoint>(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output,
	types::FixedPoint::DataType const *source)
{
	EmitCopyCodeValue<types::FixedPoint>(context, output, source);
}

} // namespace oddf::simulator::common::backend::blocks
