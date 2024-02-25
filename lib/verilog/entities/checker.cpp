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

	Verilog code emission for the Checker block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void Checker::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	std::string nrstName = exporter->GetConfiguration().negatedResetPinName;

	int numberOfInputs = (int)entity.inputs.size();

	f << "// " << entity.name << "\n";

	f << "always @(negedge clk)\n";
	f << "if (" << nrstName << " & TB_enable) begin\n";

	for (int i = 0; i < numberOfInputs; ++i)
	{
		if (!entity.inputs[i].driver->type.IsClass(dfx::types::TypeDescription::FixedPoint) && !entity.inputs[i].driver->type.IsClass(dfx::types::TypeDescription::Boolean))
		{
			dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.inputs[i].driver->type.ToString() + "'.");
			f << "// Error: code generation is not supported for type '" + entity.inputs[i].driver->type.ToString() + "'.\n\n";
			f << "end\n";
			return;
		}

		int upper_bit = entity.properties.GetInt("BitWidth") + entity.properties.GetInt("StartIndex") - 1;
		int lower_bit = entity.properties.GetInt("StartIndex");
		int bits = entity.properties.GetInt("BitWidth");
		assert(upper_bit >= lower_bit);
		assert(lower_bit >= 0);
		if (bits > 1) assert(entity.properties.GetInt("BitWidth") == entity.inputs[i].driver->type.GetWordWidth());
		f << "\tif (" << GetNodeExpression(entity.inputs[i].driver) << " !== " << "TB_current_reference_bits[" << upper_bit << ((bits > 1) ? (":" + std::to_string(lower_bit)) : "") << ("]) \n");
		f << "\tbegin\n";
		f << "\t\t$display(\"ERROR: check failed at line %1d. Signal " << GetNodeExpression(entity.inputs[i].driver)
			<< " actual value %0" << bits << "b does not match reference value %0" << bits << "b.\", TB_line_count, " << GetNodeExpression(entity.inputs[i].driver)
			<< ", " << "TB_current_reference_bits[" << upper_bit << ((bits > 1) ? (":" + std::to_string(lower_bit)) : "") << ("]);\n");
		f << "\t\tTB_error_count = TB_error_count + 1;\n";
		f << "\tend\n";
		f << "\telse begin\n";
		f << "\t\tTB_good_count = TB_good_count + 1;\n";
		f << "\tend\n";
	}
	f << "end\n";

	f << "\n";
}

}
