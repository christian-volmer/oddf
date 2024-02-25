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

	Verilog code emission for the BitCompose block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void BitCompose::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	using dfx::types::TypeDescription;

	f << "// " << entity.name << "\n";

	int numberOfInputs = (int)entity.inputs.size();

	assert((int)entity.outputs.size() == 1);
	auto output = entity.outputs[0];
	if (!output.type.IsClass(TypeDescription::FixedPoint))
	{
		dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + output.type.ToString() + "'.");
		f << "// Error: code generation is not supported for type '" + output.type.ToString() + "'.\n\n";
		return;
	}

	assert(numberOfInputs == entity.outputs[0].type.GetWordWidth());

	for (int i = 0; i < numberOfInputs; ++i) {
		auto input = *entity.inputs[i].driver;
		//	f << "assign " << GetNodeExpression(&output) << "[" << i << "] = " << GetNodeExpression(&input) << ";\n";
		f << "assign " << GetNodeExpression(&output);
		if (numberOfInputs > 1) f << "[" << i << "] = ";
		else                    f <<              " = ";
		f << GetNodeExpression(&input) << ";\n";
	}

	f << "\n";

}

}
