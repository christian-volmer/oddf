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

#include "../Delay.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

DelayMaster::DelayMaster(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string DelayMaster::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

void DelayMaster::Elaborate(ISimulatorElaborationContext &context)
{
	auto inputs = GetInputsList();
	auto inputsCount = inputs->GetSize();

	auto outputs = GetOutputsList();
	auto outputsCount = outputs->GetSize();

	// The numbers of inputs and outputs must match
	if (outputsCount != inputsCount)
		throw Exception(ExceptionCode::Unexpected);

	/*

	Elaboration splits the original delay block ('DelayMaster') up into two
	separate blocks: a starting point, which acts as the source of the value
	stored in the flip-flop; and an endpoint, which accepts the value for
	storage.

	The following code detaches all original connections from the 'DelayMaster'
	and attaches them to the newly created 'DelayStartingPoint' and
	'DelayEndpoint' simulator blocks.

	*/

	for (size_t i = 0; i < inputsCount; ++i) {

		auto const &input = inputs->Item(i);
		auto const &output = outputs->Item(i);

		auto type = input.GetType();

		// The input and output types must be identical
		if (type != output.GetType())
			throw Exception(ExceptionCode::Unexpected);

		auto &endpoint = context.AddSimulatorBlock<DelayEndpoint>(GetDesignBlockReference());
		SimulatorBlockBase *startingPoint;

		switch (type.GetTypeId()) {

			case design::NodeType::BOOLEAN: {

				startingPoint = &context.AddSimulatorBlock<DelayStartingPoint<types::Boolean>>(
					GetDesignBlockReference(), type, endpoint);
				break;
			}

			case design::NodeType::FIXED_POINT: {

				startingPoint = &context.AddSimulatorBlock<DelayStartingPoint<types::FixedPoint>>(
					GetDesignBlockReference(), type, endpoint);
				break;
			}

			default:
				throw Exception(ExceptionCode::NotImplemented);
		}

		context.TransferConnectivity(inputs->Item(i), endpoint.GetInputsList()->Item(0));
		context.TransferConnectivity(outputs->Item(i), startingPoint->GetOutputsList()->Item(0));
	}

	context.RemoveThisBlock();
}

} // namespace oddf::simulator::common::backend::blocks
