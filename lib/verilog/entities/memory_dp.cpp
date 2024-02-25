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

	Verilog code emission for dual-port memory.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void MemoryDualPort::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	std::string nrstName = exporter->GetConfiguration().negatedResetPinName;

	// order of inputs:
	//  clkEnableInput
	//  rdAddressInput
	//	wrAddressInput
	//	wrDataEnable
	//	wrDataInput

	// order of outputs:
	//	rdDataOutput

	int depth = entity.properties.GetInt("Depth");
	int numwords = entity.properties.GetInt("Width");
	std::string memname = entity.name + "_mem";

	auto clkEnableInput = *entity.inputs[0].driver;

	auto rdAddressInput = *entity.inputs[1].driver;
	auto wrAddressInput = *entity.inputs[2].driver;
	auto wrEnableInput = *entity.inputs[3].driver;
	auto inputs = &entity.inputs[4];

	assert((numwords + 4) == (int)entity.inputs.size());
	assert(numwords == (int)entity.outputs.size());
	assert(word_width(rdAddressInput.type) == word_width(wrAddressInput.type));
	assert(rdAddressInput.type.IsClass(dfx::types::TypeDescription::FixedPoint));
	assert(rdAddressInput.type.IsClass(dfx::types::TypeDescription::FixedPoint));
	assert(wrEnableInput.type.IsClass(dfx::types::TypeDescription::Boolean));

	// Determine type class and word width of data.
	dfx::types::TypeDescription::Class dataTypeClass = entity.outputs[0].type.GetClass();
	int wordwidth = word_width(entity.outputs[0].type);

	f << "// " << entity.name << "\n";

	if (dataTypeClass != dfx::types::TypeDescription::Boolean && dataTypeClass != dfx::types::TypeDescription::FixedPoint) {

		dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.outputs[0].type.ToString() + "'.");
		f << "// Error: code generation is not supported for type '" + entity.outputs[0].type.ToString() + "'.\n\n";
		return;
	}

	// Check that all data inputs and outputs have the same type class.
	for (int i = 0; i < numwords; ++i) {

		assert(inputs[i].driver->type.IsClass(dataTypeClass));
		assert(word_width(inputs[i].driver->type) == wordwidth);
		assert(entity.outputs[i].type.IsClass(dataTypeClass));
		assert(word_width(entity.outputs[i].type) == wordwidth);
	}

	int totalwidth = numwords * wordwidth;
	
	// todo - this could go into the top part
	f << "var logic " << width2string(totalwidth) << memname << "[0:" << depth - 1 << "]; //" << " width = " << totalwidth << ", depth = " << depth << "\n";
	
	f << "always @(posedge clk or negedge " << nrstName << ")\n";
	f << "begin\n";
	f << "  if (~" << nrstName << ") begin : " << entity.name << "_ResetBlock\n";
	f << "    integer i;\n";
	f << "    for (i = 0; i < " << depth << "; i = i + 1) begin\n";
	f << "      " << memname << "[i] <= "<< totalwidth << "'d0;\n";
	f << "    end\n";
	f << "  end\n";
	f << "  else begin\n";
	f << "    if (" << GetNodeExpression(&clkEnableInput) << ") begin\n";
	f << "      if (" << GetNodeExpression(&wrEnableInput) << ") begin\n";

	for (int i = 0; i < numwords; ++i) {
		auto input = *inputs[i].driver;
		f << "        " << memname << "[" << GetNodeExpression(&wrAddressInput) << "]";
		if (numwords > 1) f << "[" << (wordwidth > 1 ? (std::to_string((i + 1) * wordwidth - 1) + ":") : std::string("")) <<  i * wordwidth << "]";
		f << " <= " << GetNodeExpression(&input) << ";\n";
	}

	f << "      end\n";

	for (int i = 0; i < numwords; ++i)
	{
		auto output = entity.outputs[i];
		f << "      " << GetNodeExpression(&output) << " <= " << memname << "[" << GetNodeExpression(&rdAddressInput) << "]"; 
		if (numwords > 1) f << "[" << (wordwidth > 1 ? (std::to_string((i + 1) * wordwidth - 1) + ":") : std::string("")) << i * wordwidth << "]";
		f << ";\n";
	}

	f << "    end\n";
	f << "  end\n";
	f << "end\n";
	f << "\n";
}

}
