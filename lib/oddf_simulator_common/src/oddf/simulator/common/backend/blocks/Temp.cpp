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

#include "Temp.h"

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend::blocks {

TempMaster::TempMaster(design::blocks::backend::IDesignBlock const &designBlock) :
	SimulatorBlockBase(designBlock)
{
}

std::string TempMaster::GetDesignPathHint() const
{
	return GetDesignBlockReference()->GetPath().ToString();
}

void TempMaster::Elaborate(ISimulatorElaborationContext &context)
{
	// Temp blocks become disconnected during elaboration and should eventually
	// remove themselves.

	if (!HasConnections())
		context.RemoveThisBlock();
}

void TempMaster::GenerateCode(ISimulatorCodeGenerationContext & /* context */)
{
	throw Exception(ExceptionCode::Fail, "The simulator encountered a temporary design block that is still connected to other blocks in the design. Did you forget to assign a `forward_node`?");
}

} // namespace oddf::simulator::common::backend::blocks
