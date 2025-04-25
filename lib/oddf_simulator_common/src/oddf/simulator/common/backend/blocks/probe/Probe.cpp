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

#include "../Probe.h"

#include "ProbeAccessObject.h"

#include <oddf/design/blocks/backend/ITaggedBlock.h>
#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

Probe::Probe(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string Probe::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

void Probe::Elaborate(ISimulatorElaborationContext &)
{
	auto outputs = GetOutputsList();

	if (outputs.GetSize() != 0)
		throw Exception(ExceptionCode::Unsupported);

	auto inputs = GetInputsList();

	if (inputs.GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	auto typeId = inputs[0].GetType().GetTypeId();

	switch (typeId) {

		case design::NodeType::BOOLEAN:
		case design::NodeType::FIXED_POINT:
			break;

		default:
			throw Exception(ExceptionCode::Unsupported);
	}
}

void Probe::Finalise(ISimulatorFinalisationContext &context)
{
	/*

	We create a global ProbeAccessObject in the simulator with
	object name ':probes/<parent path of probe block>/<probe tag>'

	*/

	auto &probeBlock = GetDesignBlockReference()->GetInterface<design::blocks::backend::ITaggedBlock>();

	auto blockPath = this->GetDesignBlockReference()->GetPath().Parent();
	auto objectName = ":probes" + blockPath.Append(probeBlock.GetTag()).ToString();

	auto const &input = GetInputsList()[0];

	switch (input.GetType().GetTypeId()) {

		case design::NodeType::BOOLEAN: {

			context.ConstructGlobalObject<ProbeAccessObject<types::Boolean>>(
				objectName,
				context.GetCurrentComponent(),
				input.GetDriver());
			break;
		}

		case design::NodeType::FIXED_POINT: {

			context.ConstructGlobalObject<ProbeAccessObject<types::FixedPoint>>(
				objectName,
				context.GetCurrentComponent(),
				input.GetDriver());
			break;
		}

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

} // namespace oddf::simulator::common::backend::blocks
