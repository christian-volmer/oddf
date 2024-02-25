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

	Verilog code emission for module outputs.

*/

#include "../global.h"
#include "entities.h"

namespace entities {


void OutputPort::WritePreamble(std::ofstream &, dfx::generator::Instance &, dfx::generator::Entity &) const
{
	assert(false);
}

void OutputPort::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	if (entity.inputs.size() > 1) {

		std::string expression = GetArrayExpression(&entity.inputs[0], 0, (int)entity.inputs.size());

		if (!expression.empty())
			f << "assign " << entity.inputs[0].name << " = " << expression << ";\n";
		else {
			for (auto const &input : entity.inputs)
				f << "assign " << input.name << "[" << input.index << "] = " << GetNodeExpression(input.driver) << ";\n";
		}
	}
	else
		f << "assign " << entity.inputs[0].name << " = " << GetNodeExpression(entity.inputs[0].driver) << ";\n";
}

}
