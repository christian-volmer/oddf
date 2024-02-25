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

	Verilog code emission for the Delay block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

	using namespace dfx;

	void Delay::WritePreamble(std::ofstream &f, generator::Instance &, generator::Entity &entity) const
	{
		for (auto &output : entity.outputs) {
			if (output.busIndex == 0)
				f << "var logic" << signal_declaration(output.type, output.GetBaseName(), output.busSize, ";") << "\n";
		}
	}

	void Delay::WriteCode(std::ofstream &f, generator::Instance &, generator::Entity &entity) const
	{
		std::string nrstName = exporter->GetConfiguration().negatedResetPinName;

		// Number of outputs is equal to the number of data lines ("paths")
		int numberOfPaths = (int)entity.outputs.size();

		// Check if our delay element has a clock enable input
		bool hasClockEnable = numberOfPaths + 1 == (int)entity.inputs.size();
		assert(numberOfPaths + (hasClockEnable ? 1 : 0) == (int)entity.inputs.size());

		// If we have a clock enable input, it is the first input and the data inputs are after it.
		generator::Entity::Input *clockEnableInput = hasClockEnable ? &entity.inputs[0] : nullptr;
		auto dataInputs = entity.inputs.begin() + (hasClockEnable ? 1 : 0);

		assert(!hasClockEnable || clockEnableInput->driver->type.IsClass(types::TypeDescription::Boolean));

		types::TypeDescription type = entity.outputs[0].type;

		f << "// " << entity.name << "\n";
		f << "always @(posedge clk or negedge " << nrstName << ")\n";
		f << "begin\n";
		f << "\tif (~" << nrstName << ") begin\n";

		for (int i = 0; i < numberOfPaths; ++i)
		{
			switch (entity.outputs[i].type.GetClass()) {
				case types::TypeDescription::Boolean:
				{
					f << "\t\t" << GetNodeExpression(&entity.outputs[i]) << " <= 1'b0;\n";
					break;
				}
				case types::TypeDescription::FixedPoint:
				{
					f << "\t\t" << GetNodeExpression(&entity.outputs[i]) << " <= " << std::to_string(entity.outputs[i].type.GetWordWidth()) << "'d0;\n";
					break;
				}
				default:
					dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.outputs[i].type.ToString() + "'.");
					f << "\t// Error: code generation is not supported for type '" + entity.outputs[i].type.ToString() + "'.\n\n";
					f << "\tend\nend\n";
					return;
			}
		}

		f << "\tend\n";

		if (hasClockEnable)
			f << "\telse if (" << GetNodeExpression(clockEnableInput->driver) << ") begin\n";
		else
			f << "\telse begin\n";

		for (int i = 0; i < numberOfPaths; ++i) {

			auto const &driver = *dataInputs[i].driver;
			auto const &output = entity.outputs[i];

			assert(output.type == driver.type);

			f << "\t\t" << GetNodeExpression(&output) << " <= " << GetNodeExpression(&driver) << ";\n";
		}

		f << "\tend\n";
		f << "end\n\n";
	}
}
