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

	Verilog code emission for the FloorCast block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void FloorCast::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	int numberOfInputs = (int)entity.inputs.size();
	int numberOfOutputs = (int)entity.outputs.size();

	assert(numberOfOutputs == numberOfInputs);

	f << "// " << entity.name << "\n";

	for (int i = 0; i < numberOfOutputs; ++i)
	{
		if (!entity.outputs[i].type.IsClass(dfx::types::TypeDescription::FixedPoint))
		{
			dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.outputs[i].type.ToString() + "'.");
			f << "// Error: code generation is not supported for type '" + entity.outputs[i].type.ToString() + "'.\n\n";
			return;
		}
		if (!entity.inputs[i].driver->type.IsClass(dfx::types::TypeDescription::FixedPoint))
		{
			dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.inputs[i].driver->type.ToString() + "'.");
			f << "// Error: code generation is not supported for type '" + entity.inputs[i].driver->type.ToString() + "'.\n\n";
			return;
		}

		assert(entity.outputs[i].type.GetWordWidth() > 0);
		assert(entity.inputs[i].driver->type.GetWordWidth() > 0);

		int i_signed = entity.inputs[i].driver->type.IsSigned();
		int o_width = entity.outputs[i].type.GetWordWidth();
		int i_width = entity.inputs[i].driver->type.GetWordWidth();
		int o_nonfr = entity.outputs[i].type.GetWordWidth() - entity.outputs[i].type.GetFraction();
		int i_nonfr = entity.inputs[i].driver->type.GetWordWidth() - entity.inputs[i].driver->type.GetFraction();
		int o_fract = entity.outputs[i].type.GetFraction();
		int i_fract = entity.inputs[i].driver->type.GetFraction();

		int signext = o_nonfr - i_nonfr; 
		int zeropad = o_fract - i_fract;
		int left_idx = signext > 0 ? i_width - 1 : i_width - 1 + signext;
		int right_idx = zeropad >= 0 ? 0 : -zeropad;
		int overbits = (signext < 0) ? -signext : 0;
		
		if (left_idx < right_idx) {
			if (zeropad > 0) {
				zeropad = o_width;
				overbits = i_width;
			}
			if (signext > 0) {
				signext = o_width;
				overbits = 0;
			}
		}

		int concatenate = 0;

		std::string rhs = "";

		// handle sign extension
		if (signext > 0) 
		{
			if (concatenate++ > 0) rhs += ", ";

			if (i_signed)
			{
				// signed values
				if (signext == 1) 
				{
					rhs += GetNodeExpression(entity.inputs[i].driver) + "[" + std::to_string(i_width - 1) + "]";
				}
				else 
				{
					rhs += "{" + std::to_string(signext) + "{" + GetNodeExpression(entity.inputs[i].driver) + "[" + std::to_string(i_width - 1) + "]}}";
				}
			}
			// unsigned values
			else 
			{
				rhs += std::to_string(signext) + "'d0";
			}
		}

		// handle input signal 
		if (left_idx >= right_idx) 
		{

			if (concatenate++ > 0) rhs += ", ";

			rhs += GetNodeExpression(entity.inputs[i].driver);

			if (left_idx != (i_width - 1) || right_idx != 0) 
			{
				if (left_idx == right_idx)
				{
					rhs += "[" + std::to_string(left_idx) + "]";
				}
				else
				{
					rhs += "[" + std::to_string(left_idx) + ":" + std::to_string(right_idx) + "]";
				}
			}
		}

		// do zero padding 
		if (zeropad > 0) 
		{
			if (concatenate++ > 0) rhs += ", ";

			rhs += std::to_string(zeropad) + "'d0";
		}

		if (concatenate > 1) 
		{
			rhs = "{" + rhs + "}";
		}

		f << "assign " << GetNodeExpression(&entity.outputs[i]) << " = " << rhs;
		f << "; // cast " << entity.inputs[i].driver->type.ToString() << " to " << entity.outputs[i].type.ToString() << "\n";

		//f << "// assign overflow = ";
		//
		//if (overbits == 0)
		//{
		//	f << "1'b0;\n";
		//}
		//else if (overbits > 0)
		//{
		//	if (i_signed) {
		//		f << entity.outputs[i].name << "[" << i_width - 1 - overbits << "] ";
		//		
		//		if (overbits == 1)
		//		{
		//			f << "^ ~" << entity.outputs[i].name << "[" << i_width - 1 << "];\n";
		//		}
		//		else
		//		{
		//			f << "? ~&" << entity.outputs[i].name << "[" << i_width - 1 << ":" << i_width - overbits << "] :\n"
		//				 << "|" << entity.outputs[i].name << "[" << i_width - 1 << ":" << i_width - overbits << "]; \n";
		//		}

		//	}
		//	else
		//	{
		//		if (overbits == 0)
		//		{
		//			f << entity.outputs[i].name << "[" << i_width - 1 << "];\n";
		//		}
		//		else 
		//		{
		//			f << "|" << entity.outputs[i].name << "[" << i_width - 1 << ":" << i_width - overbits - 1 << "];\n";
		//		}
		//	}
		//}

		
		
	}

	f << "\n";
}

}
