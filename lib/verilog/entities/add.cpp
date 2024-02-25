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

	Verilog code emission for addition.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

	void Add::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
	{
		int numberOfInputs = (int)entity.inputs.size();
		int numberOfOutputs = (int)entity.outputs.size();

		assert(numberOfInputs % numberOfOutputs == 0);

		int numberOfInputsPerOutput = numberOfInputs / numberOfOutputs;

		f << "// " << entity.name << "\n";

		for (int i = 0; i < numberOfOutputs; ++i) 
		{
			auto output = entity.outputs[i];
			if (!output.type.IsClass(dfx::types::TypeDescription::FixedPoint))
			{
				dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + output.type.ToString() + "'.");
				f << "// Error: code generation is not supported for type '" + output.type.ToString() + "'.\n\n";
				return;
			}
			
			int outputWidth = output.type.GetWordWidth();
			f << "assign " << GetNodeExpression(&output) << " = ";

			std::string typelist = "// " + output.type.ToString() + " = ";

			for (int j = 0; j < numberOfInputsPerOutput; ++j) 
			{
				auto input = entity.inputs[numberOfInputsPerOutput*i + j].driver;

				int right = output.type.GetFraction() - input->type.GetFraction();
				int left  = outputWidth - input->type.GetWordWidth() - right;
				assert(right >= 0);
				assert(left >= 0);
				assert(left + right + input->type.GetWordWidth() == outputWidth);
				
				f << expand_signal(input, left, right);

				if (j != numberOfInputsPerOutput - 1)
				{
					f << " + ";
					typelist += input->type.ToString() + " + ";
				}
				else
				{
					f << "; " << typelist << input->type.ToString() + "\n";
				}
			}
		}

		f << "\n";
	}
}
