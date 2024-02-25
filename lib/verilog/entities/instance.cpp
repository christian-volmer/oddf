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

	Verilog code emission for module instantiations.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void Instance::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	std::string nrstName = exporter->GetConfiguration().negatedResetPinName;

	f << "// " << entity.name << "\n";

	int inputIndex = 0;
	std::list<std::string> inputExpressions;
	for (auto &inputPort : entity.targetModule->inputPorts) {

		int portSize = (int)inputPort->outputs.size();

		if (portSize == 1)
			inputExpressions.push_back(GetNodeExpression(entity.inputs[inputIndex]));
		else {

			std::string expression = GetArrayExpression(&entity.inputs[0], inputIndex, portSize);

			if (expression.empty()) {

				// Elements of this bus input cannot be represented as a simple expression. Create temporary bus and assign elements individually.
				std::string busName = entity.name + "_" + inputPort->outputs[0].GetBaseName();
				f << "var logic" << signal_declaration(inputPort->outputs[0].type, busName, inputPort->outputs[0].busSize, ";") << "\n";

				for (int i = 0; i < portSize; ++i)
					f << "assign " << busName << "[" << i << "] = " << GetNodeExpression(entity.inputs[inputIndex + i].driver) << ";\n";

				f << "\n";

				expression = busName;

			}

			inputExpressions.push_back(expression);
		}

		inputIndex += portSize;
	}

	f << entity.targetModule->moduleName << " " << entity.name << "\n(\n";
	f << "\t.clk(clk),\n";
	f << "\t." << nrstName << "(" << nrstName << "),";

	bool first = true;
	auto expressionIt = inputExpressions.begin();
	for (auto &inputPort : entity.targetModule->inputPorts) {

		f << (first ? "\n" : ",\n");
		f << "\t." << inputPort->outputs[0].GetBaseName() << "(" << *expressionIt << ")";
		++expressionIt;
		first = false;
	}

	int outputIndex = 0;
	for (auto &outputPort : entity.targetModule->outputPorts) {

		f << (first ? "\n" : ",\n");
		f << "\t." << outputPort->inputs[0].name << "(" << entity.outputs[outputIndex].GetBaseName() << ")";
		outputIndex += (int)outputPort->inputs.size();
		first = false;
	}

	f << "\n);\n\n";

}

}
