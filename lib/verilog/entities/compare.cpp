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

	Verilog code emission for arithmetic comparison.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void Compare::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	int numberOfInputs = (int)entity.inputs.size();
	int numberOfOutputs = (int)entity.outputs.size();

	assert(numberOfInputs == 2*numberOfOutputs);

	f << "// " << entity.name << "\n";

	for (int i = 0; i < numberOfOutputs; ++i) {
		
		f << "assign " << GetNodeExpression(&entity.outputs[i]) << " = ";

		auto input1 = entity.inputs[2 * i + 0].driver;
		auto input2 = entity.inputs[2 * i + 1].driver;
		
		assert(input1->type.GetClass() == input2->type.GetClass());
		assert(entity.outputs[i].type.GetClass() == dfx::types::TypeDescription::Boolean);

		switch (input1->type.GetClass())
		{
			case dfx::types::TypeDescription::FixedPoint:
			{
				// TODO: something is wrong with the needsign-logic
				// TODO: move $signed stuff into GetExpandedNodeExpression() and use that here?

				bool needsign = input1->type.IsSigned() || input2->type.IsSigned();
				int add1 = !input1->type.IsSigned() && input2->type.IsSigned() ? 1 : 0;
				int add2 = !input2->type.IsSigned() && input1->type.IsSigned() ? 1 : 0;
				int fraction = std::max(input1->type.GetFraction(), input2->type.GetFraction());
				int width = std::max(input1->type.GetWordWidth() - input1->type.GetFraction() + add1, input2->type.GetWordWidth() - input2->type.GetFraction()) + fraction + add2;

				int right = fraction - input1->type.GetFraction();
				int left = width - input1->type.GetWordWidth() - right;
				assert(right >= 0);
				assert(left >= 0);
				assert(left + right + input1->type.GetWordWidth() == width);

				bool alreadySigned1 = input1->type.IsSigned() && (left == 0) && (right == 0);
				f << (needsign && !alreadySigned1 ? "$signed(" : "") << expand_signal(input1, left, right) << (needsign && !alreadySigned1 ? ")" : "");
				f << " " << operation << " ";

				right = fraction - input2->type.GetFraction();
				left = width - input2->type.GetWordWidth() - right;
				assert(right >= 0);
				assert(left >= 0);
				assert(left + right + input2->type.GetWordWidth() == width);

				bool alreadySigned2 = input2->type.IsSigned() && (left == 0) && (right == 0);
				f << (needsign && !alreadySigned2 ? "$signed(" : "") << expand_signal(input2, left, right) << (needsign && !alreadySigned2 ? ")" : "");
				f << "; //" << input1->type.ToString() << " " << operation << " " << input2->type.ToString() << "\n";
				break;
			}

			case dfx::types::TypeDescription::Boolean:
			{
				f << GetNodeExpression(input1) << " " << operation << " " << GetNodeExpression(input2) << "; //";
				f << input1->type.ToString() << " " << operation << " " << input2->type.ToString() << "\n";

				break;
			}

			default: 
				dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + input1->type.ToString() + "'.");
				f << "1'bx; // Error: code generation is not supported for type '" + input1->type.ToString() + "'.\n\n";
				return;
		}
	}

	f << "\n";
}

}
