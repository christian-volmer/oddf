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
#include "BooleanFlatCode.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

enum class BooleanFlat::OperationName {

	UNDEFINED = 0,
	AND = 1,
	OR = 2,
	XOR = 3
};

BooleanFlat::BooleanFlat(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock),
	m_operationName(OperationName::UNDEFINED),
	m_busIndex(-1)
{
	auto blockClass = designBlock.GetClass();

	if (blockClass == std::string("and"))
		m_operationName = OperationName::AND;
	else if (blockClass == std::string("or"))
		m_operationName = OperationName::OR;
	else if (blockClass == std::string("xor"))
		m_operationName = OperationName::XOR;
	else
		throw Exception(ExceptionCode::Unexpected);
}

BooleanFlat::BooleanFlat(design::blocks::backend::IDesignBlock const *designBlock, OperationName operationName, size_t numberOfInputs, ptrdiff_t busIndex) :
	SimulatorBlockBase(designBlock, numberOfInputs, { design::NodeType::Boolean() }),
	m_operationName(operationName),
	m_busIndex(busIndex)
{
}

std::string BooleanFlat::GetDesignPathHint() const
{
	if (m_busIndex >= 0)
		return GetDesignBlockReference()->GetPath().ToString() + "<" + std::to_string(m_busIndex) + ">";
	else
		return GetDesignBlockReference()->GetPath().ToString();
}

void BooleanFlat::Elaborate(ISimulatorElaborationContext &context)
{
	auto inputs = GetInputsList();
	auto inputsCount = inputs->GetSize();

	auto outputs = GetOutputsList();
	auto outputsCount = outputs->GetSize();

	if (outputsCount == 0) {

		if (inputsCount == 0) {

			context.RemoveThisBlock();
			return;
		}

		throw Exception(ExceptionCode::Unexpected);
	}

	/*

	We support Boolean operations with more than two operands. And we support
	busses, in which case there will be more than one output. Every bus element
	must do the same operation, so the number of inputs must be an integer
	multiple of the number of outputs. We check this below.

	*/

	if (inputsCount % outputsCount != 0)
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

	    TODO: introduce simulator options (elaboration option to control the behaviour)

	*/

	if (outputsCount > 1) {

		auto inputsPerOperator = inputsCount / outputsCount;

		for (size_t i = 0; i < outputsCount; ++i) {

			auto &newBlock = context.AddSimulatorBlock<BooleanFlat>(GetDesignBlockReference(), m_operationName, inputsPerOperator, i);

			for (size_t j = 0; j < inputsPerOperator; ++j)
				context.TransferConnectivity(inputs->Item(inputsPerOperator * i + j), newBlock.GetInputsList()->Item(j));

			context.TransferConnectivity(outputs->Item(i), newBlock.GetOutputsList()->Item(0));
		}

		context.RemoveThisBlock();
		return;
	}

	if (!HasConnections()) {

		context.RemoveThisBlock();
		return;
	}

	for (size_t i = 0; i < inputsCount; ++i)
		if (inputs->Item(i).GetType().GetTypeId() != design::NodeType::BOOLEAN)
			throw Exception(ExceptionCode::Unexpected);

	for (size_t i = 0; i < outputsCount; ++i)
		if (outputs->Item(i).GetType().GetTypeId() != design::NodeType::BOOLEAN)
			throw Exception(ExceptionCode::Unexpected);
}

void BooleanFlat::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	size_t inputsCount = GetInputsList()->GetSize();
	size_t outputsCount = GetOutputsList()->GetSize();

	switch (m_operationName) {

		case OperationName::AND:
			EmitBooleanAndCode(context, inputsCount, outputsCount);
			break;

		case OperationName::OR:
			EmitBooleanOrCode(context, inputsCount, outputsCount);
			break;

		case OperationName::XOR:
			EmitBooleanXorCode(context, inputsCount, outputsCount);
			break;

		default:
			throw Exception(ExceptionCode::Unexpected);
	}
}

} // namespace oddf::simulator::common::backend::blocks
