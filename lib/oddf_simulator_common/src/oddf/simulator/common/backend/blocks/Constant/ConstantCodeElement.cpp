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

#include "ConstantCode.h"

#include <oddf/simulator/common/backend/Types.h>

namespace oddf::simulator::common::backend::blocks {

template<typename T>
struct ConstantInstructionElement : public SimulatorInstruction {

	T m_output;

	static void InstructionFunction(ConstantInstructionElement *)
	{
	}
};

template<typename T>
void EmitConstantCodeElement(ISimulatorCodeGenerationContext &context, IListView<SimulatorBlockOutput const &> &outputs,
	design::blocks::backend::IConstantBlock const &constantBlock, ptrdiff_t constantIndex)
{
	size_t outputsCount = outputs.GetSize();

	/*
	    `constantIndex` == -1 : index of the constant equals the index of the
	                            output
	    `constantIndex` >=  0 : index of the constant is as given.
	                            `outputsCount` must equal 1 in this case.
	*/

	if (constantIndex >= 0 && outputsCount != 1)
		throw Exception(ExceptionCode::Unexpected);

	for (size_t i = 0; i < outputsCount; ++i) {

		auto &output = outputs.Item(i);

		auto nodeType = output.GetType();
		auto elementCount = T::GetElementCount(nodeType);

		size_t outputIndex = output.GetIndex();

		context.StartInstructionWithOutput(ConstantInstructionElement<T>::InstructionFunction,
			&ConstantInstructionElement<T>::m_output, elementCount);
		auto *instruction = context.CommitInstruction<ConstantInstructionElement<T>>();

		context.BindOutput(outputIndex, instruction->m_output);
		constantBlock.Read(constantIndex >= 0 ? constantIndex : outputIndex, instruction->m_output.GetData(), T::GetDataSize(nodeType));

		if (!instruction->m_output.CheckIntegrity(nodeType))
			throw Exception(ExceptionCode::Unexpected);
	}
}

template<>
void EmitConstantCode<types::FixedPoint>(ISimulatorCodeGenerationContext &context, IListView<SimulatorBlockOutput const &> &outputs,
	design::blocks::backend::IConstantBlock const &constantBlock, ptrdiff_t constantIndex)
{
	EmitConstantCodeElement<types::FixedPoint>(context, outputs, constantBlock, constantIndex);
}

} // namespace oddf::simulator::common::backend::blocks
