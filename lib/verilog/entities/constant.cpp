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

	Verilog code emission for the Constant block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

using namespace dfx;

void Constant::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	using dfx::types::TypeDescription;

	int numberOfOutputs = (int)entity.outputs.size();

	assert(numberOfOutputs == entity.properties.GetInt("BusWidth"));

	f << "// " << entity.name << "\n";

	for (int i = 0; i < numberOfOutputs; ++i) {

		auto const &output = entity.outputs[i];

		switch (output.type.GetClass()) {

			case TypeDescription::Boolean: {

				f << "assign " << GetNodeExpression(&output) << " = 1'b" << entity.properties.GetInt("Constant", i) << ";\n";
				break;
			}

			case TypeDescription::FixedPoint: {

				int constantData[dynfix::MAX_FIELDS];
				int width = output.type.GetWordWidth();
				int fraction = output.type.GetFraction();

				entity.properties.GetIntArray("Constant", i, constantData);
				f << "assign " << GetNodeExpression(&output) << " = " << width << "'b";

				for (int i = width - 1; i >= 0; --i) {
					f << std::to_string((constantData[i / 32] >> (i & 0x1F)) & 0x1);
					if (fraction != 0 && i == fraction) f << "_";
				}

				f << "; // ";

				// Convert to double representation
				double value = 0.0;
				if (output.type.IsSigned() && (constantData[dynfix::MAX_FIELDS - 1] < 0)) {

					value = 1.0;
					for (int i = 0; i < dynfix::MAX_FIELDS; ++i)
						value += ~(unsigned)constantData[i] * std::pow(2.0, i * 32);

					value = -value;
				}
				else {

					for (int i = 0; i < dynfix::MAX_FIELDS; ++i)
						value += (unsigned)constantData[i] * std::pow(2.0, i * 32);
				}

				value *= std::pow(2.0, -output.type.GetFraction());

				f << output.type.ToString() << " (" << value << ")\n";

				break;
			}

			default:
				dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + output.type.ToString() + "'.");
				f << "// Error: code generation is not supported for type '" + output.type.ToString() + "'.\n\n";
				return;
		}
	}

	f << "\n";
}

}
