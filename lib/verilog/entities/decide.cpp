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

	Verilog code emission for the Decide block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void Decide::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	using dfx::types::TypeDescription;

	int numberOfInputs = (int)entity.inputs.size();
	int numberOfOutputs = (int)entity.outputs.size();

	assert(numberOfInputs == 2*numberOfOutputs+1);

	auto decideInput = entity.inputs[0];
	auto trueFalseInputs = entity.inputs.begin() + 1;

	f << "// " << entity.name << "\n";

	for (int i = 0; i < numberOfOutputs; ++i) {

		auto trueDriver = trueFalseInputs[i * 2].driver;
		auto falseDriver = trueFalseInputs[i * 2 + 1].driver;

		switch (entity.outputs[i].type.GetClass()) {
			case TypeDescription::Boolean:
			{
				f << "assign " << GetNodeExpression(&entity.outputs[i]) << " = "
					<< GetNodeExpression(decideInput.driver) << " ? "
					<< GetNodeExpression(trueDriver) << " : "
					<< GetNodeExpression(falseDriver) << ";\n";
				break;
			}

			case TypeDescription::FixedPoint:
			{
				int true_width = trueDriver->type.GetWordWidth();
				int false_width = falseDriver->type.GetWordWidth();
				int width = entity.outputs[i].type.GetWordWidth();
				int true_right = entity.properties.GetInt("TrueShift", i);
				int false_right = entity.properties.GetInt("FalseShift", i);
				int true_left = width - true_width - true_right;
				int false_left = width - false_width - false_right;

				f << "assign " << GetNodeExpression(&entity.outputs[i]) << " = "
					<< GetNodeExpression(decideInput.driver) << " ? "
				  << expand_signal(trueDriver, true_left, true_right) << " : "
				  << expand_signal(falseDriver, false_left, false_right) << ";\n";
				break;
			}
			default:
				dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.outputs[i].type.ToString() + "'.");
				f << "// Error: code generation is not supported for type '" + entity.outputs[i].type.ToString() + "'.\n\n";
				return;
		}
	}

	f << "\n";
}

}
