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

	Verilog code emission for the stimuli of the Recorder block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void Stimulus::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	int numberOfOutputs = (int)entity.inputs.size();

	f << "// " << entity.name << "\n";

	for (int i = 0; i < numberOfOutputs; ++i)
	{
		if (!entity.outputs[i].type.IsClass(dfx::types::TypeDescription::FixedPoint) && !entity.inputs[i].driver->type.IsClass(dfx::types::TypeDescription::Boolean))
		{
			dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.inputs[i].driver->type.ToString() + "'.");
			f << "// Error: code generation is not supported for type '" + entity.inputs[i].driver->type.ToString() + "'.\n\n";
			f << "end\n";
			return;
		}

		int bits = entity.properties.GetInt("BitWidth");
		int upper_bit = bits + entity.properties.GetInt("StartIndex") - 1;
		int lower_bit = entity.properties.GetInt("StartIndex");
		assert(upper_bit >= lower_bit);
		assert(lower_bit >= 0);
		if (bits > 1) assert(bits == entity.inputs[i].driver->type.GetWordWidth());
		f << "assign " << GetNodeExpression(&entity.outputs[i]) << " = " << "TB_current_stimulus_bits[" << upper_bit << ((bits > 1) ? (":" + std::to_string(lower_bit)) : "") << ("];\n");
	}

	f << "\n";
}

}
