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

#include "../Constant.h"

#include <oddf/design/blocks/backend/IConstantBlock.h>

#include <oddf/simulator/common/backend/types/CheckFixedPointRepresentation.h>

#include <oddf/Exception.h>

#include <cassert>

namespace oddf::simulator::common::backend::blocks {

ConstantMaster::ConstantMaster(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string ConstantMaster::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

void ConstantMaster::Elaborate(ISimulatorElaborationContext &)
{
	auto outputs = GetOutputsList();

	if (outputs->GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	auto inputs = GetInputsList();

	if (inputs->GetSize() != 0)
		throw Exception(ExceptionCode::Unsupported);

	auto typeId = outputs->Item(0).GetType().GetTypeId();

	if (!((typeId == design::NodeType::BOOLEAN)
			|| (typeId == design::NodeType::FIXED_POINT)))
		throw Exception(ExceptionCode::Unsupported);
}

template<typename T, typename = void>
class ConstantInstruction;

template<typename T>
class ConstantInstruction<T, std::void_t<typename T::ValueType>> : public SimulatorInstruction {

private:

	T m_output;

	static void InstructionFunction(ConstantInstruction *)
	{
	}

public:

	static void Emit(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, design::blocks::backend::IConstantBlock const &constantBlock)
	{
		context.StartInstruction(InstructionFunction);
		auto *instruction = context.CommitInstruction<ConstantInstruction>();

		context.BindOutput(output.GetIndex(), instruction->m_output);
		constantBlock.Read(&instruction->m_output, sizeof(T));
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

	static void Emit(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, design::blocks::backend::IConstantBlock const &constantBlock)
	{
		auto nodeType = output.GetType();
		auto elementCount = T::RequiredElementCount(nodeType);

		context.StartInstructionWithOutput(InstructionFunction, &ConstantInstruction::m_output, elementCount);
		auto *instruction = context.CommitInstruction<ConstantInstruction>();

		context.BindOutput(output.GetIndex(), instruction->m_output);
		constantBlock.Read(instruction->m_output.m_elements, sizeof(typename T::ElementType) * elementCount);

		if (!types::CheckFixedPointRepresentation(instruction->m_output, nodeType))
			throw Exception(ExceptionCode::Unexpected);
	};
};

void EmitConstantInstruction(ISimulatorCodeGenerationContext &context, SimulatorBlockOutput const &output, design::blocks::backend::IConstantBlock const &constantBlock)
{
	switch (output.GetType().GetTypeId()) {

		case design::NodeType::BOOLEAN: {

			ConstantInstruction<types::Boolean>::Emit(context, output, constantBlock);
			break;
		}

		case design::NodeType::FIXED_POINT: {

			ConstantInstruction<types::FixedPoint>::Emit(context, output, constantBlock);
			break;
		}

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

void ConstantMaster::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	auto &constantBlock = GetDesignBlockReference()->GetInterface<design::blocks::backend::IConstantBlock>();

	EmitConstantInstruction(context, GetOutputsList()->Item(0), constantBlock);
}

} // namespace oddf::simulator::common::backend::blocks
