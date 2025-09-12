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
#include "NotCode.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

Not::Not(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock),
	m_busIndex(-1)
{
}

Not::Not(design::blocks::backend::IDesignBlock const *designBlock, ptrdiff_t busIndex) :
	SimulatorBlockBase(designBlock, 1, { design::NodeType::Boolean() }),
	m_busIndex(busIndex)
{
}

std::string Not::GetDesignPathHint() const
{
	if (m_busIndex >= 0)
		return GetDesignBlockReference()->GetPath().ToString() + "<" + std::to_string(m_busIndex) + ">";
	else
		return GetDesignBlockReference()->GetPath().ToString();
}

void Not::Elaborate(ISimulatorElaborationContext &context)
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

			auto &newBlock = context.AddSimulatorBlock<Not>(GetDesignBlockReference(), i);

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

		if (inputs->Item(i).GetType().GetTypeId() != design::NodeType::BOOLEAN)
			throw Exception(ExceptionCode::Unexpected);

		if (outputs->Item(i).GetType().GetTypeId() != design::NodeType::BOOLEAN)
			throw Exception(ExceptionCode::Unexpected);
	}
}

void Not::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	EmitNotCode(context, *GetOutputsList(), *GetInputsList());
}

} // namespace oddf::simulator::common::backend::blocks
