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

    Simulator support for equality testing.

*/

#include "../Equal.h"
#include "../FloorCast.h"
#include "EqualCode.h"

#include <oddf/Exception.h>

#include <algorithm>

namespace oddf::simulator::common::backend::blocks {

Equal::Equal(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock),
	m_typeId(),
	m_busIndex(-1)
{
}

Equal::Equal(design::blocks::backend::IDesignBlock const *designBlock, ptrdiff_t busIndex) :
	SimulatorBlockBase(designBlock, 2, { design::NodeType::Boolean() }),
	m_busIndex(busIndex)
{
}

std::string Equal::GetDesignPathHint() const
{
	if (m_busIndex >= 0)
		return GetDesignBlockReference()->GetPath().ToString() + "<" + std::to_string(m_busIndex) + ">";
	else
		return GetDesignBlockReference()->GetPath().ToString();
}

void Equal::Elaborate(ISimulatorElaborationContext &context)
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

	We support equality testing on busses and return a boolean bus in this case.

	*/

	if (inputsCount != 2 * outputsCount)
		throw Exception(ExceptionCode::Unexpected);

	// See comment in the `Elaborate` function of the `BooleanFlat` class.
	if (outputsCount > 1) {

		for (size_t i = 0; i < outputsCount; ++i) {

			auto &newBlock = context.AddSimulatorBlock<Equal>(GetDesignBlockReference(), i);

			context.TransferConnectivity(inputs->Item(2 * i + 0), newBlock.GetInputsList()->Item(0));
			context.TransferConnectivity(inputs->Item(2 * i + 1), newBlock.GetInputsList()->Item(1));

			context.TransferConnectivity(outputs->Item(i), newBlock.GetOutputsList()->Item(0));
		}

		context.RemoveThisBlock();
		return;
	}

	if (!HasConnections()) {

		context.RemoveThisBlock();
		return;
	}

	m_typeId = inputs->Item(0).GetType().GetTypeId();
	if (inputs->Item(1).GetType().GetTypeId() != m_typeId)
		throw Exception(ExceptionCode::Unexpected);

	switch (m_typeId) {

		case design::NodeType::BOOLEAN:
			// For boolean operands, elaboration finishes here
			break;

		case design::NodeType::FIXED_POINT:
			ElaborateFixedPoint(context);
			break;

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

void Equal::ElaborateFixedPoint(ISimulatorElaborationContext &context)
{
	auto inputs = GetInputsList();

	// The core comparison instruction expects the types of both operands
	// to be identical. We insert floor-cast blocks here if necessary.

	// First, we determine the common type that can represent values from both inputs.

	// TODO: generalise for more than 2 inputs, make a helper function?

	auto type0 = inputs->Item(0).GetType();
	auto type1 = inputs->Item(1).GetType();
	bool sign = type0.IsSigned() || type1.IsSigned();

	int msbPosition = std::max(
		type0.GetWordWidth() - type0.GetFraction() - type0.IsSigned(),
		type1.GetWordWidth() - type1.GetFraction() - type1.IsSigned());

	int lsbPosition = std::min(-type0.GetFraction(), -type1.GetFraction());

	auto commonType = design::NodeType::FixedPoint(sign, msbPosition - lsbPosition + sign, -lsbPosition);

	// Walk through the inputs and insert a floor-cast block, if necessary

	auto inputEnumerator = inputs->GetEnumerator();
	while (inputEnumerator->MoveNext()) {

		auto &theInput = inputEnumerator->GetCurrent();

		if (theInput.GetType() != commonType) {

			auto &inputCastBlock = context.AddSimulatorBlock<FloorCast>(
				GetDesignBlockReference(),
				commonType, -1);

			context.TransferConnectivity(theInput, inputCastBlock.GetInputsList()->GetFirst());
			context.Connect(inputCastBlock.GetOutputsList()->GetFirst(), theInput);
		}
	}
}

void Equal::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	switch (m_typeId) {

		case design::NodeType::BOOLEAN:
			EmitEqualCode<types::Boolean>(context, GetOutputsList()->GetFirst(), *GetInputsList());
			break;

		case design::NodeType::FIXED_POINT:
			EmitEqualCode<types::FixedPoint>(context, GetOutputsList()->GetFirst(), *GetInputsList());
			break;

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

} // namespace oddf::simulator::common::backend::blocks
