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

	Verilog code emission for multiplication.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

//int g_maxOperandWidth = 0;

void Mul::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	using dfx::types::TypeDescription;

	int numberOfInputs = (int)entity.inputs.size();
	int numberOfOutputs = (int)entity.outputs.size();

	assert(numberOfInputs % numberOfOutputs == 0);

	int numberOfInputsPerOutput = numberOfInputs / numberOfOutputs;

	f << "// " << entity.name << "\n";

	dfx::types::TypeDescription::Class dataTypeClass = entity.outputs[0].type.GetClass();

	if (dataTypeClass != dfx::types::TypeDescription::FixedPoint) {

		dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.outputs[0].type.ToString() + "'.");
		f << "// Error: code generation is not supported for type '" + entity.outputs[0].type.ToString() + "'.\n\n";
		return;
	}

	assert(numberOfInputsPerOutput == 2);

	//int maxOperandWidth = 0;
	
	for (int i = 0; i < numberOfOutputs; ++i) {
		auto output = entity.outputs[i];
		assert(output.type.IsClass(dataTypeClass));

		int useSigned = output.type.IsSigned();
		if (useSigned == 0) {	// check if any of the inputs is signed which is not allowed in this case
			for (int j = 0; j < numberOfInputsPerOutput; ++j) {
				assert(entity.inputs[numberOfInputsPerOutput*i + j].driver->type.IsSigned() == 0);
			}
		}

		f << "assign " << GetNodeExpression(&output) << " = ";
		std::string typelist = "// " + output.type.ToString() + " = ";

		for (int j = 0; j < numberOfInputsPerOutput; ++j) {
			auto input = entity.inputs[numberOfInputsPerOutput*i + j].driver;
			assert(input->type.IsClass(dataTypeClass));

			//maxOperandWidth = std::max(maxOperandWidth, input->type.GetWordWidth());

			// check if we need a typecast to signed
			if (input->type.IsSigned() == 0 && useSigned != 0) {
				f << "$signed({1'b0," << GetNodeExpression(input) << "})";
			}
			else {
				f << GetNodeExpression(input);
			}

			if (j != numberOfInputsPerOutput - 1) {
				f << " * ";
				typelist += input->type.ToString() + " * ";
			}
			else {
				f << "; " << typelist << input->type.ToString() << "\n";
			}
		}
	}

	f << "\n";

	/*if (maxOperandWidth > g_maxOperandWidth) {

		g_maxOperandWidth = maxOperandWidth;

		std::cout << "\n" << entity.inputs[0].driver->type.ToString() << " * " << entity.inputs[1].driver->type.ToString() << " = " << entity.outputs[0].type.ToString() << "\n\n"

		}*/
}
}
