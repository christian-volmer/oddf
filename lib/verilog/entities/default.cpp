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

	Fall-back Verilog code emission for unsupported blocks.

*/

#include "../global.h"
#include "entities.h"

namespace entities {


void Default::WritePreamble(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	for (auto &output : entity.outputs) {
		if (output.busIndex == 0)
			f << "var logic" << signal_declaration(output.type, output.GetBaseName(), output.busSize, ";") << "\n";
	}
}

void Default::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	dfx::design_info("Block '" + entity.name + "': code generation is not supported for this block.");
	f << "// name = '" << entity.name << "', class = '" << entity.className << "' number of inputs = " << entity.inputs.size() << ", number of outputs = " << entity.outputs.size() << "\n";
	f << "// Error: code generation is not supported for this block.\n\n";
}

}
