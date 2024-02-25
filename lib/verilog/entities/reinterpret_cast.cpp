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

	Verilog code emission for the ReinterpretCast block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void ReinterpretCast::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	int numberOfInputs = (int)entity.inputs.size();
	int numberOfOutputs = (int)entity.outputs.size();

	assert(numberOfOutputs == numberOfInputs);

	f << "// " << entity.name << "\n";

	for (int i = 0; i < numberOfOutputs; ++i) {

		auto output = entity.outputs[i];
		auto input = *entity.inputs[i].driver;

		if ((!output.type.IsClass(dfx::types::TypeDescription::Boolean) && !output.type.IsClass(dfx::types::TypeDescription::FixedPoint))
			|| (!input.type.IsClass(dfx::types::TypeDescription::Boolean) && !input.type.IsClass(dfx::types::TypeDescription::FixedPoint))) {

			dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + output.type.ToString() + "'.");
			f << "// Error: code generation is not supported for type '" + output.type.ToString() + "'.\n\n";
			return;
		}
	}

	for (int i = 0; i < numberOfOutputs; ++i)
	{
		auto output = entity.outputs[i];
		auto input = *entity.inputs[i].driver;

		int outwidth = word_width(output.type);
		int inwidth = word_width(input.type);

		assert(inwidth > 0);
		assert(outwidth > 0);

		f << "assign " << GetNodeExpression(&output) << " = ";

		if (inwidth > outwidth)
		{
			f << GetNodeExpression(&input) << "[" << outwidth - 1 << ((outwidth > 1) ? ":0]" : "]");
		}
		else if (inwidth == outwidth)
		{
			f << GetNodeExpression(&input);
		}
		else
		{
			f << "{" << outwidth - inwidth << "'d0, " << GetNodeExpression(&input) << "}";
		}

		f << "; // reinterpret " << input.type.ToString() << " as " << output.type.ToString() << "\n";
	}

	f << "\n";
}

}
