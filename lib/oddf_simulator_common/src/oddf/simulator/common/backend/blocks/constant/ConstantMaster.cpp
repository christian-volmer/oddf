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
#include "I_Const_Bool.h"
#include "I_Const_FixedPoint.h"

#include <oddf/Exception.h>
#include <oddf/design/blocks/backend/IConstantBlock.h>

namespace oddf::simulator::common::backend::blocks {

ConstantMaster::ConstantMaster(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string ConstantMaster::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath();
}

void ConstantMaster::Elaborate(ISimulatorElaborationContext &)
{
	auto outputs = GetOutputsList();

	if (outputs.GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	auto inputs = GetInputsList();

	if (inputs.GetSize() != 0)
		throw Exception(ExceptionCode::Unsupported);

	auto typeId = outputs[0].GetType().GetTypeId();

	if (!((typeId == design::NodeType::BOOLEAN)
			|| (typeId == design::NodeType::FIXED_POINT)))
		throw Exception(ExceptionCode::Unsupported);
}

void ConstantMaster::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	auto &constantBlock = GetDesignBlockReference()->GetInterface<design::blocks::backend::IConstantBlock>();

	auto outputType = GetOutputsList()[0].GetType();
	switch (outputType.GetTypeId()) {

		case design::NodeType::BOOLEAN: {

			context.EmitInstruction<I_Const_Bool>(constantBlock);
			break;
		}

		case design::NodeType::FIXED_POINT: {

			size_t elementCount = types::FixedPointElement::RequiredElementCount(outputType);

			context.EmitInstructionVariadic<I_Const_FixedPoint>(
				I_Const_FixedPoint::GetVariadicMember(), elementCount,
				constantBlock, elementCount);
			break;
		}

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

} // namespace oddf::simulator::common::backend::blocks
