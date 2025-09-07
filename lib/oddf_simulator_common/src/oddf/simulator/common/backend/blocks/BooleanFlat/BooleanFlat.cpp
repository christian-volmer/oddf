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

    Simulator support for the Boolean operators AND, OR, and XOR.

*/

#include "../BooleanFlat.h"

#include <oddf/Exception.h>

#include <cassert>

namespace oddf::simulator::common::backend::blocks {

// Explicit template implementation of AND, OR, and XOR

template class BooleanFlat<std::logical_and<bool>, true>;
template class BooleanFlat<std::logical_or<bool>, false>;
template class BooleanFlat<std::not_equal_to<bool>, false>;

template<typename functionT, bool identityValue>
BooleanFlat<functionT, identityValue>::BooleanFlat(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

template<typename functionT, bool identityValue>
BooleanFlat<functionT, identityValue>::BooleanFlat(design::blocks::backend::IDesignBlock const *designBlock, size_t numberOfInputs) :
	SimulatorBlockBase(designBlock, numberOfInputs, { design::NodeType::Boolean() })
{
}

template<typename functionT, bool identityValue>
std::string BooleanFlat<functionT, identityValue>::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

template<typename functionT, bool identityValue>
void BooleanFlat<functionT, identityValue>::Elaborate(ISimulatorElaborationContext &context)
{
	auto inputs = GetInputsList();
	auto inputsCount = inputs->GetSize();

	auto outputs = GetOutputsList();
	auto outputsCount = outputs->GetSize();

	/*

	We support Boolean operations with more than two operands. And we support
	busses, in which case there will be more than one output. Every bus element
	must do the same operation, so the number of inputs must be an integer
	multiple of the number of outputs. We check this below.

	*/

	if (outputsCount == 0 || (inputsCount % outputsCount != 0))
		throw Exception(ExceptionCode::Unexpected);

	/*

	There are three different ways how we can deal with busses:

	  1. Split this bus operation up into individual blocks, one for each output.
	     Do this during elaboration so that each block find its optimum place
	     in the execution graph. This is done below.

	  2. Do not split up the operation during elaboration but emit multiple pieces
	     of code, one for each output. Speeds up elaboration and the creation of
	     the execution graph, but since all operations remain tied together in a
	     single block, the execution graph may be inferiour. On the other hand, there
	     will be fewer components, which can speed things up. This is already
	     implemented and can be benchmarked by simply commenting out the following
	     block of code.

	  3. Create a single piece of code that can handle all cases. Might save some
	     memory because the output values of large busses can be stored in a
	     contiguous block of bytes without any alignment padding. This has not been
	     implemented yet.

	*/

	if (outputsCount > 1) {

		auto inputsPerOperator = inputsCount / outputsCount;

		for (size_t i = 0; i < outputsCount; ++i) {

			auto &block = context.AddSimulatorBlock<BooleanFlat>(GetDesignBlockReference(), inputsPerOperator);

			for (size_t j = 0; j < inputsPerOperator; ++j)
				context.TransferConnectivity(inputs->Item(inputsPerOperator * i + j), block.GetInputsList()->Item(j));

			context.TransferConnectivity(outputs->Item(i), block.GetOutputsList()->Item(0));
		}

		context.RemoveThisBlock();
		return;
	}

	for (auto outputsEnumerator = outputs->GetEnumerator(); outputsEnumerator->MoveNext();)
		if (outputsEnumerator->GetCurrent().GetType().GetTypeId() != design::NodeType::BOOLEAN)
			throw Exception(ExceptionCode::Unexpected);

	for (auto inputsEnumerator = inputs->GetEnumerator(); inputsEnumerator->MoveNext();)
		if (inputsEnumerator->GetCurrent().GetType().GetTypeId() != design::NodeType::BOOLEAN)
			throw Exception(ExceptionCode::Unexpected);
}

template<typename functionT, bool identityValue>
struct BooleanFlatInstruction : public SimulatorInstruction {

	types::Boolean m_result;
	size_t m_operandCount;
	types::Boolean const *m_operands[1];

private:

	void Operate()
	{
		bool result = identityValue;
		for (size_t i = 0; i < m_operandCount; ++i)
			result = functionT {}(result, m_operands[i]->m_value != 0);

		m_result.m_value = result;
	}

	static void InstructionFunction(BooleanFlatInstruction *instruction)
	{
		instruction->Operate();
	}

public:

	static void Emit(ISimulatorCodeGenerationContext &context, size_t outputIndex, size_t inputsStartingIndex, size_t inputsCount)
	{
		context.StartInstructionVariadic(BooleanFlatInstruction::InstructionFunction, &BooleanFlatInstruction::m_operands, inputsCount);
		auto *instruction = context.CommitInstruction<BooleanFlatInstruction>();

		instruction->m_operandCount = inputsCount;

		for (size_t i = 0; i < inputsCount; ++i)
			context.BindInputReference(inputsStartingIndex + i, instruction->m_operands[i]);

		context.BindOutput(outputIndex, instruction->m_result);
	}
};

template<typename functionT, bool identityValue>
void BooleanFlat<functionT, identityValue>::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	size_t inputsCount = GetInputsList()->GetSize();
	size_t outputsCount = GetOutputsList()->GetSize();

	auto inputsPerOperator = inputsCount / outputsCount;

	for (size_t i = 0; i < outputsCount; ++i)
		BooleanFlatInstruction<functionT, identityValue>::Emit(context, i, inputsPerOperator * i, inputsPerOperator);
}

} // namespace oddf::simulator::common::backend::blocks
