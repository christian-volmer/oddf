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

#include "../Plus.h"
#include "PlusCode.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

Plus::Plus(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock),
	m_busIndex(-1)
{
}

Plus::Plus(design::blocks::backend::IDesignBlock const *designBlock, design::NodeType const &outputType,
	size_t numberOfInputs, ptrdiff_t busIndex) :
	SimulatorBlockBase(designBlock, numberOfInputs, { outputType }),
	m_busIndex(busIndex)
{
}

std::string Plus::GetDesignPathHint() const
{
	if (m_busIndex >= 0)
		return GetDesignBlockReference()->GetPath().ToString() + "<" + std::to_string(m_busIndex) + ">";
	else
		return GetDesignBlockReference()->GetPath().ToString();
}

void Plus::Elaborate(ISimulatorElaborationContext &context)
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

	Our Plus operation supports more than two operands. And we support
	busses, in which case there will be more than one output. Every bus element
	must do the same operation, so the number of inputs must be an integer
	multiple of the number of outputs. We check this below.

	*/

	if (inputsCount % outputsCount != 0)
		throw Exception(ExceptionCode::Unexpected);

	if (outputsCount > 1) {

		auto inputsPerOutput = inputsCount / outputsCount;

		for (size_t i = 0; i < outputsCount; ++i) {

			auto const &output = outputs->Item(i);

			auto &newBlock = context.AddSimulatorBlock<Plus>(GetDesignBlockReference(), output.GetType(), inputsPerOutput, i);

			for (size_t j = 0; j < inputsPerOutput; ++j)
				context.TransferConnectivity(inputs->Item(inputsPerOutput * i + j), newBlock.GetInputsList()->Item(j));

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
		if (inputs->Item(i).GetType().GetTypeId() != design::NodeType::FIXED_POINT)
			throw Exception(ExceptionCode::Unexpected);

	for (size_t i = 0; i < outputsCount; ++i)
		if (outputs->Item(i).GetType().GetTypeId() != design::NodeType::FIXED_POINT)
			throw Exception(ExceptionCode::Unexpected);
}

void Plus::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	EmitPlusCode(context, *GetOutputsList(), *GetInputsList());
}

} // namespace oddf::simulator::common::backend::blocks
