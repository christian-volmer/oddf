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

	Verilog code emission for the BitExtract block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void BitExtract::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	using dfx::types::TypeDescription;

	f << "// " << entity.name << "\n";

	int numberOfOutputs = (int)entity.outputs.size();

	if (!entity.inputs[0].driver->type.IsClass(TypeDescription::FixedPoint))
	{
		dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.inputs[0].driver->type.ToString() + "'.");
		f << "// Error: code generation is not supported for type '" + entity.inputs[0].driver->type.ToString() + "'.\n\n";
		return;
	}

	int firstBitIndex = entity.properties.GetInt("FirstBitIndex");
	int lastBitIndex = entity.properties.GetInt("LastBitIndex");
	int increment = firstBitIndex < lastBitIndex ? 1 : -1;

	assert((int)entity.inputs.size() == 1);
	assert(numberOfOutputs == 1 + increment * (lastBitIndex - firstBitIndex) );

	int index = firstBitIndex;
	for (int i = 0; i < numberOfOutputs; ++i) {
		f << "assign " << GetNodeExpression(&entity.outputs[i]) << " = " << GetNodeExpression(entity.inputs[0].driver) << "[" << index << "];\n";
		index += increment;
	}

	f << "\n";

}

}
