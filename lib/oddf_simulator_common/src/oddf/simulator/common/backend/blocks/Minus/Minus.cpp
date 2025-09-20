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

#include "../Minus.h"
#include "MinusCode.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

Minus::Minus(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock),
	m_busIndex(-1)
{
}

Minus::Minus(design::blocks::backend::IDesignBlock const *designBlock, design::NodeType const &type, ptrdiff_t busIndex) :
	SimulatorBlockBase(designBlock, 1, { type }),
	m_busIndex(busIndex)
{
}

std::string Minus::GetDesignPathHint() const
{
	if (m_busIndex >= 0)
		return GetDesignBlockReference()->GetPath().ToString() + "<" + std::to_string(m_busIndex) + ">";
	else
		return GetDesignBlockReference()->GetPath().ToString();
}

void Minus::Elaborate(ISimulatorElaborationContext &context)
{
	auto inputs = GetInputsList();
	auto inputsCount = inputs->GetSize();

	auto outputs = GetOutputsList();
	auto outputsCount = outputs->GetSize();

	if (inputsCount != outputsCount)
		throw Exception(ExceptionCode::Unexpected);

	// See comment in the `Elaborate` function of the `BooleanFlat` class.
	if (outputsCount > 1) {

		for (size_t i = 0; i < outputsCount; ++i) {

			auto &newBlock = context.AddSimulatorBlock<Minus>(GetDesignBlockReference(), outputs->Item(i).GetType(), i);

			context.TransferConnectivity(inputs->Item(i), newBlock.GetInputsList()->Item(0));
			context.TransferConnectivity(outputs->Item(i), newBlock.GetOutputsList()->Item(0));
		}

		context.RemoveThisBlock();
		return;
	}

	if (!HasConnections()) {

		context.RemoveThisBlock();
		return;
	}

	for (size_t i = 0; i < outputsCount; ++i) {

		if (inputs->Item(i).GetType().GetTypeId() != design::NodeType::FIXED_POINT)
			throw Exception(ExceptionCode::NotImplemented);

		if (outputs->Item(i).GetType().GetTypeId() != design::NodeType::FIXED_POINT)
			throw Exception(ExceptionCode::NotImplemented);
	}
}

void Minus::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	EmitMinusCode(context, *GetOutputsList(), *GetInputsList());
}

} // namespace oddf::simulator::common::backend::blocks
