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

	Verilog code emission for arithmetic negation.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void Negate::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	int numberOfInputs = (int)entity.inputs.size();
	int numberOfOutputs = (int)entity.outputs.size();

	assert(numberOfOutputs == numberOfInputs);

	f << "// " << entity.name << "\n";

	dfx::types::TypeDescription::Class dataTypeClass = entity.outputs[0].type.GetClass();

	if (dataTypeClass != dfx::types::TypeDescription::FixedPoint) {

		dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.outputs[0].type.ToString() + "'.");
		f << "// Error: code generation is not supported for type '" + entity.outputs[0].type.ToString() + "'.\n\n";
		return;
	}

	for (int i = 0; i < numberOfOutputs; ++i) {

		auto output = entity.outputs[i];
		assert(output.type.IsClass(dataTypeClass));

		f << "assign " << GetNodeExpression(&output)
			<< " = -{";
		
		if (entity.inputs[i].driver->type.IsSigned())
			f << GetNodeExpression(entity.inputs[i].driver)
			<< "[" << entity.inputs[i].driver->type.GetWordWidth() - 1 << "], ";
		else
			f << " 1'b0, ";
		
		f << GetNodeExpression(entity.inputs[i].driver) << "};\n";
	}

	f << "\n";
}

}
