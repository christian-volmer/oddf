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

	Verilog code emission for boolean operations.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void Logic::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	int numberOfInputs = (int)entity.inputs.size();
	int numberOfOutputs = (int)entity.outputs.size();

	assert(numberOfInputs % numberOfOutputs == 0);
	int numberOfInputsPerOutput = numberOfInputs / numberOfOutputs;

	f << "// " << entity.name << "\n";

	for (int i = 0; i < numberOfOutputs; ++i) {

		assert(entity.outputs[i].type.IsClass(dfx::types::TypeDescription::Boolean));

		f << "assign " << GetNodeExpression(&entity.outputs[i]) << " = ";
		for (int j = 0; j < numberOfInputsPerOutput; ++j) {

			assert(entity.inputs[i].driver->type.IsClass(dfx::types::TypeDescription::Boolean));

			f << GetNodeExpression(entity.inputs[numberOfInputsPerOutput*i + j].driver);
			if (j != numberOfInputsPerOutput - 1) {
				f << " " << operation << " ";
			}
			else {
				f << ";\n";
			}
		}
	}

	f << "\n";
}

}
