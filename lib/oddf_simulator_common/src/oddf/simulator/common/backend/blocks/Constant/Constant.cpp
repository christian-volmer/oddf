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

    Simulator support for the 'constant' design block.

*/

#include "../Constant.h"

#include <oddf/design/blocks/backend/IConstantBlock.h>

#include <oddf/Exception.h>

#include <cassert>

namespace oddf::simulator::common::backend::blocks {

Constant::Constant(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock),
	m_typeId()
{
}

std::string Constant::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

void Constant::Elaborate(ISimulatorElaborationContext &context)
{
	// The 'constant' block cannot have any inputs.
	if (GetInputsList()->GetSize() != 0)
		throw Exception(ExceptionCode::Unexpected);

	auto outputs = GetOutputsList();
	auto outputsCount = outputs->GetSize();

	if (outputsCount == 0) {

		// TODO issue warning?
		context.RemoveThisBlock();
		return;
	}

	m_typeId = outputs->Item(0).GetType().GetTypeId();

	// We currently support only `Boolean` and `FixedPoint`
	if (!((m_typeId == design::NodeType::BOOLEAN)
			|| (m_typeId == design::NodeType::FIXED_POINT)))
		throw Exception(ExceptionCode::Unsupported);

	// All outputs must have the same type-id
	for (size_t i = 1; i < outputsCount; ++i)
		if (outputs->Item(i).GetType().GetTypeId() != m_typeId)
			throw Exception(ExceptionCode::Unsupported);

	// Currently, we support multiple outputs only with the Boolean type.
	if (outputsCount > 1 && m_typeId != design::NodeType::BOOLEAN)
		throw Exception(ExceptionCode::NotImplemented);
}

template<typename T, typename = void>
class ConstantInstruction;

template<typename T>
class ConstantInstruction<T, std::void_t<typename T::ValueType>> : public SimulatorInstruction {

private:

	T m_outputs[1];

	static void InstructionFunction(ConstantInstruction *)
	{
	}

public:

	static void Emit(ISimulatorCodeGenerationContext &context, IListView<SimulatorBlockOutput const &> &outputs, design::blocks::backend::IConstantBlock const &constantBlock)
	{
		size_t outputsCount = outputs.GetSize();

		context.StartInstructionVariadic(InstructionFunction, &ConstantInstruction::m_outputs, outputsCount);
		auto *instruction = context.CommitInstruction<ConstantInstruction>();

		for (size_t i = 0; i < outputsCount; ++i) {

			context.BindOutput(i, instruction->m_outputs[i]);
			constantBlock.Read(i, instruction->m_outputs[i].GetData(), T::GetDataSize());
		}
	};
};

template<typename T>
class ConstantInstruction<T, std::void_t<typename T::ElementType>> : public SimulatorInstruction {

private:

	T m_output;

	static void InstructionFunction(ConstantInstruction *)
	{
	}

public:

	static void Emit(ISimulatorCodeGenerationContext &context, IListView<SimulatorBlockOutput const &> &outputs, design::blocks::backend::IConstantBlock const &constantBlock)
	{
		// More than one output is currently not implemented for variable sized
		// simulator types.
		if (outputs.GetSize() > 1)
			throw Exception(ExceptionCode::NotImplemented);

		auto &output = outputs.Item(0);

		auto nodeType = output.GetType();
		auto elementCount = T::GetElementCount(nodeType);

		context.StartInstructionWithOutput(InstructionFunction, &ConstantInstruction::m_output, elementCount);
		auto *instruction = context.CommitInstruction<ConstantInstruction>();

		context.BindOutput(output.GetIndex(), instruction->m_output);
		constantBlock.Read(0, instruction->m_output.GetData(), T::GetDataSize(nodeType));

		if (!instruction->m_output.CheckIntegrity(nodeType))
			throw Exception(ExceptionCode::Unexpected);
	};
};

void Constant::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	auto &constantBlock = GetDesignBlockReference()->GetInterface<design::blocks::backend::IConstantBlock>();

	switch (m_typeId) {

		case design::NodeType::BOOLEAN: {

			ConstantInstruction<types::Boolean>::Emit(context, *GetOutputsList(), constantBlock);
			break;
		}

		case design::NodeType::FIXED_POINT: {

			ConstantInstruction<types::FixedPoint>::Emit(context, *GetOutputsList(), constantBlock);
			break;
		}

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

} // namespace oddf::simulator::common::backend::blocks
