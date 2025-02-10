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

#include "../Signal.h"

#include "SignalAccessObject.h"

#include "../../instructions/Copy.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

Signal::Signal(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string Signal::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath();
}

void Signal::Elaborate(ISimulatorElaborationContext &)
{
	auto outputs = GetOutputsList();

	if (outputs.GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	auto typeId = outputs[0].GetType().GetTypeId();

	switch (typeId) {

		case design::NodeType::BOOLEAN:
		case design::NodeType::FIXED_POINT:
			break;

		default:
			throw Exception(ExceptionCode::Unsupported);
	}

	auto inputs = GetInputsList();

	if (inputs.GetSize() != 0)
		throw Exception(ExceptionCode::Unsupported);
}

void Signal::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	auto outputs = GetOutputsList();
	auto const &output = outputs[0];

	auto type = output.GetType();

	switch (type.GetTypeId()) {

		case design::NodeType::BOOLEAN: {

			auto &signalAccessObject = context.ConstructGlobalObject<SignalAccessObject<types::Boolean>>("mysignal",
				context.GetCurrentComponent(), type);

			context.EmitInstruction<instructions::Copy<types::Boolean>>(signalAccessObject.GetSource());

			break;
		}

		case design::NodeType::FIXED_POINT: {

			auto &signalAccessObject = context.ConstructGlobalObject<SignalAccessObject<types::FixedPointElement>>("mysignal",
				context.GetCurrentComponent(), type);

			size_t elementCount = types::FixedPointElement::RequiredElementCount(type);

			context.EmitInstructionVariadic<instructions::Copy<types::FixedPointElement>>(
				instructions::Copy<types::FixedPointElement>::GetVariadicMember(), elementCount,
				signalAccessObject.GetSource(), elementCount);

			break;
		}

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

} // namespace oddf::simulator::common::backend::blocks
