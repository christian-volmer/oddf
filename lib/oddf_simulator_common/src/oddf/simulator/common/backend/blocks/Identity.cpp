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

    Simulator support for the 'identity' design block.

*/

#include "Identity.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

Identity::Identity(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string Identity::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

void Identity::Elaborate(ISimulatorElaborationContext &context)
{
	auto inputs = GetInputsList();
	auto outputs = GetOutputsList();

	if (inputs.GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (outputs.GetSize() != 1)
		throw Exception(ExceptionCode::Unsupported);

	if (inputs[0].GetType() != outputs[0].GetType())
		throw Exception(ExceptionCode::Unexpected);

	context.TransferConnectivity(outputs[0], inputs[0].GetDriver());
	context.DisconnectInput(inputs[0]);
	context.RemoveThisBlock();
}

} // namespace oddf::simulator::common::backend::blocks
