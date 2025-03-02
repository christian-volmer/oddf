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
	return GetDesignBlockReference()->GetPath();
}

void DelayMaster::Elaborate(ISimulatorElaborationContext &context)
{
	auto outputs = GetOutputsList();

	size_t pathCount = outputs.GetSize();

	if (pathCount != 1)
		throw Exception(ExceptionCode::Unsupported);

	auto type = outputs[0].GetType();

	if (!((type.GetTypeId() == design::NodeType::BOOLEAN)
			|| (type.GetTypeId() == design::NodeType::FIXED_POINT)))
		throw Exception(ExceptionCode::Unsupported);

	for (size_t i = 1; i < pathCount; ++i)
		if (outputs[i].GetType() != type)
			throw Exception(ExceptionCode::Unsupported);

	auto inputs = GetInputsList();

	if (inputs.GetSize() != pathCount)
		throw Exception(ExceptionCode::Unsupported);

	for (size_t i = 0; i < pathCount; ++i)
		if (inputs[i].GetType() != type)
			throw Exception(ExceptionCode::Unsupported);

	/*

	Elaboration splits the original delay block ('DelayMaster') up into two
	separate blocks: a starting point, which acts as the source of the value
	stored in the flip-flop; and an endpoint, which accepts the value for
	storage.

	The following code detaches all original connections from the 'DelayMaster'
	and attaches them to the newly created 'DelayStartingPoint' and
	'DelayEndpoint' simulator blocks.

	*/

	auto &endpoint = context.AddSimulatorBlock<DelayEndpoint>(GetDesignBlockReference());

	SimulatorBlockBase *startingPoint;

	switch (type.GetTypeId()) {

		case design::NodeType::BOOLEAN: {

			startingPoint = &context.AddSimulatorBlock<DelayStartingPoint<types::Boolean>>(
				GetDesignBlockReference(),
				type,
				endpoint);
			break;
		}

		case design::NodeType::FIXED_POINT: {

			startingPoint = &context.AddSimulatorBlock<DelayStartingPoint<types::FixedPointElement>>(
				GetDesignBlockReference(),
				type,
				endpoint);
			break;
		}

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}

	context.TransferConnectivity(inputs[0], endpoint.GetInputsList()[0]);
	context.TransferConnectivity(outputs[0], startingPoint->GetOutputsList()[0]);

	context.RemoveThisBlock();
}

} // namespace oddf::simulator::common::backend::blocks
