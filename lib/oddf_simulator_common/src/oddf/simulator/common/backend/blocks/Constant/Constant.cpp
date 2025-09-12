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
#include "ConstantCode.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

Constant::Constant(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock),
	m_typeId(),
	m_busIndex(-1)
{
}

Constant::Constant(design::blocks::backend::IDesignBlock const *designBlock, design::NodeType const &type, ptrdiff_t busIndex) :
	SimulatorBlockBase(designBlock, 0, { type }),
	m_typeId(),
	m_busIndex(busIndex)
{
}

std::string Constant::GetDesignPathHint() const
{
	if (m_busIndex >= 0)
		return GetDesignBlockReference()->GetPath().ToString() + "<" + std::to_string(m_busIndex) + ">";
	else
		return GetDesignBlockReference()->GetPath().ToString();
}

void Constant::Elaborate(ISimulatorElaborationContext &context)
{
	// The 'constant' block cannot have any inputs.
	if (GetInputsList()->GetSize() != 0)
		throw Exception(ExceptionCode::Unexpected);

	auto outputs = GetOutputsList();
	auto outputsCount = outputs->GetSize();

	// See comment in the `Elaborate` function of the `BooleanFlat` class.
	if (outputsCount > 1) {

		for (size_t i = 0; i < outputsCount; ++i) {

			auto &newBlock = context.AddSimulatorBlock<Constant>(GetDesignBlockReference(), outputs->Item(i).GetType(), i);
			context.TransferConnectivity(outputs->Item(i), newBlock.GetOutputsList()->Item(0));
		}

		context.RemoveThisBlock();
		return;
	}

	if (!HasConnections()) {

		context.RemoveThisBlock();
		return;
	}

	m_typeId = outputs->Item(0).GetType().GetTypeId();

	// All outputs must have the same type-id
	for (size_t i = 1; i < outputsCount; ++i)
		if (outputs->Item(i).GetType().GetTypeId() != m_typeId)
			throw Exception(ExceptionCode::Unsupported);
}

void Constant::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	auto &constantBlock = GetDesignBlockReference()->GetInterface<design::blocks::backend::IConstantBlock>();

	switch (m_typeId) {

		case design::NodeType::BOOLEAN:
			EmitConstantCode<types::Boolean>(context, *GetOutputsList(), constantBlock, m_busIndex);
			break;

		case design::NodeType::FIXED_POINT:
			EmitConstantCode<types::FixedPoint>(context, *GetOutputsList(), constantBlock, m_busIndex);
			break;

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

} // namespace oddf::simulator::common::backend::blocks
