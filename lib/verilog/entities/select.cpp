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

	Verilog code emission for the Select block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void Select::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	// order of inputs:
	//  indexInput
	//	inputs

	// order of outputs:
	//	outputs

	f << "// " << entity.name << "\n";

	if (!entity.outputs[0].type.IsClass(dfx::types::TypeDescription::FixedPoint) && !entity.outputs[0].type.IsClass(dfx::types::TypeDescription::Boolean))
	{
		dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.outputs[0].type.ToString() + "'.");
		f << "// Error: code generation is not supported for type '" + entity.outputs[0].type.ToString() + "'.\n\n";
		return;
	}
	if (!entity.inputs[1].driver->type.IsClass(dfx::types::TypeDescription::FixedPoint) && !entity.inputs[1].driver->type.IsClass(dfx::types::TypeDescription::Boolean))
	{
		dfx::design_info("Block '" + entity.name + "': code generation is not supported for type '" + entity.inputs[1].driver->type.ToString() + "'.");
		f << "// Error: code generation is not supported for type '" + entity.inputs[1].driver->type.ToString() + "'.\n\n";
		return;
	}

	int numberOfInputs = (int)entity.inputs.size() - 1;
	int numberOfOutputs = (int)entity.outputs.size();

	assert(numberOfOutputs >= 1);
	assert(numberOfInputs >= numberOfOutputs);

	auto &indexInput = entity.inputs[0];
	auto *dataInputs = &entity.inputs[1];
	auto *dataOutputs = &entity.outputs[0];

	std::string outputsBaseName = dataOutputs[0].GetBaseName();
	auto outputsType = dataOutputs[0].type;

	if (numberOfInputs == 1) {

		f << "assign " << outputsBaseName << " = " << GetNodeExpression(dataInputs[0]) << ";\n";
	}
	else {

		std::string inputsBaseName;
		inputsBaseName = entity.name + "_InputArray";

		// new implementation
		int start = 0;
		if (IsInputBusOrSlice(dataInputs, 0, numberOfInputs)) 
		{
			start = dataInputs[0].driver->busIndex;
			inputsBaseName = dataInputs[0].driver->GetBaseName();
		}
		else
		{
			f << "var logic" << signal_declaration(outputsType, inputsBaseName, numberOfInputs, ";") << "\n";

			for (int i = 0; i < numberOfInputs ; ++i)
				f << "assign " << inputsBaseName << "[" << i << "] = " << GetExpandedNodeExpression(outputsType, dataInputs[i]) << ";\n";
		}

		assert(indexInput.driver->type.GetFraction() <= 0);

		int idxMsbFromIdxInput = indexInput.driver->type.GetWordWidth() - indexInput.driver->type.GetFraction() - 1; 
		int idxMsbFromNumInOut = ((int)std::ceil(std::log2(numberOfInputs - numberOfOutputs + 1))) - 1;
		int idxMsb = 0;
		if (idxMsbFromIdxInput < idxMsbFromNumInOut)
		{
			// special handling in case the index input has smaller bitwidth than actually required for the given number of inputs and outputs
			idxMsb = idxMsbFromIdxInput;
			numberOfInputs = (1 << (idxMsb+1)) + numberOfOutputs - 1;
		}
		else
		{
			idxMsb = idxMsbFromNumInOut;
		}
		int idxLsb = -indexInput.driver->type.GetFraction();
		int idxBits = idxMsb - idxLsb + 1;

		assert(idxMsb >= idxLsb);
		assert(idxBits <= indexInput.driver->type.GetWordWidth());
		assert(indexInput.driver->type.GetFraction() <= 0);

		// for debugging
		//f << "// numberOfInputs = " << numberOfInputs << ", numberOfOutputs = " << numberOfOutputs << ", idxMsb = " << idxMsb << ", idxLsb = " << idxLsb << "\n";

		for (int bit = idxMsb; bit >= idxLsb; bit--)
		{
			int step = 1 << bit;
			int numNodes = step + numberOfOutputs - (1 << idxLsb); // total number of output nodes of in this level
			int numMuxes = numberOfInputs - step;  // total number of real 2:1 multiplexers in this level
			int numThrough = numNodes - numMuxes;  // total number of simple through nodes in this level
			int level = bit - idxLsb;
			if (numMuxes > numNodes) numMuxes = numNodes; // limit number of muxes in case it should exceed the number of nodes
			
			// for debugging
			//f << "// step = " << step << ", numNodes = " << numNodes << ", numMuxes = " << numMuxes << ", level = " << level << "\n";

			std::string outputLevelName;
			if (bit == idxLsb) outputLevelName = outputsBaseName;
			else
			{
				outputLevelName = outputsBaseName + "_level_" + std::to_string(level);
				f << "var logic" << signal_declaration(outputsType, outputLevelName, numNodes, ";") << "\n";
			}

			f << "assign " << outputLevelName;
			if (numberOfOutputs > 1 || level > 0)
			{
				f << "[0";
				if (numMuxes > 1) f << ":" << numMuxes - 1;
				f << "]";
			}
			f << " = " << GetNodeExpression(indexInput);
			if (indexInput.driver->type.GetWordWidth() > 1) f << "[" << level << "]";
			f << " ? " << inputsBaseName << "[" << start + step; 
			if (numMuxes > 1) f << ":" << start + step + numMuxes - 1;
			f << "]";
			f << " : " << inputsBaseName << "[" << start; 
			if (numMuxes > 1) f << ":" << start + numMuxes - 1;
			f << "];\n";

			if (numThrough > 0)
			{
				f << "assign " << outputLevelName << "[" << numMuxes;
				if (numThrough > 1) f << ":" << numNodes - 1;
				f << "]";
				f << " = " << inputsBaseName << "[" << start + numMuxes;
				if (numThrough > 1) f << ":" << start + numNodes - 1;
				f << "];\n";
			}

			// input of next stage is output of this stage
			inputsBaseName = outputLevelName;
			numberOfInputs = numNodes;
			start = 0;
		}





#if 0
		int stride = 1 << (-indexInput.driver->type.GetFraction());
		if (exporter->GetConfiguration().useIndexedPartSelect) {

			f << "assign " << outputsBaseName << " = " << inputsBaseName << "[";

			if (start != 0)
				f << start << " + ";
			if (stride != 1)
				f << stride << " * ";

			f << GetNodeExpression(indexInput);

			if (numberOfOutputs > 1)
				f << " +: " << numberOfOutputs;

			f << "];\n";
		}
		else {

			if (numberOfOutputs > 1) {
				f << "generate\n";
				f << "for (genvar i = 0; i < " << numberOfOutputs << "; i++) ";
			}

			f << "assign " << outputsBaseName;

			if (numberOfOutputs > 1) {
				f << "[i]";
			}

			f << " = " << inputsBaseName << "[";

			if (start != 0)
				f << start << " + ";

			if (stride != 1)
				f << stride << " * ";

			f << GetNodeExpression(indexInput);

			if (numberOfOutputs > 1) {
				f << " + i];\n";
				f << "endgenerate\n";
			}
			else {
				f << "];\n";
			}
		}
#endif
	}

#if 0

	std::string sel_name = entity.name + "_inputarray";

	auto index = *entity.inputs[0].driver;
	auto inputs = &entity.inputs[1];
	auto outputs = entity.outputs;

	int num_out = entity.outputs.size();
	int num_in = entity.inputs.size() - 1;
	int width = word_width(entity.outputs[0].type);
	int fract = fraction(entity.outputs[0].type);

	assert(num_in > num_out);
	assert(fraction(index.type) <= 0);
	for (int i = 0; i < num_out; ++i)
		assert(word_width(outputs[i].type) == width);


#if 1

	// todo - this could go into the top part
	f << "var logic " << width2string(width) << sel_name << "[0:" << num_in - 1 << "];\n";

	// connect input vector
	for (int i = 0; i < num_in; ++i)
	{
		auto input = *inputs[i].driver;

		int right = fract - fraction(input.type);
		int left = width - word_width(input.type) - right;
		assert(right >= 0);
		assert(left >= 0);
		assert(left + right + word_width(input.type) == width);

		f << "assign " << sel_name << "[" << i << "] = " << expand_signal(&input, left, right) << ";\n";
	}

	// connect outputs
	for (int i = 0; i < num_out; ++i)
	{
		auto output = outputs[i];
		f << "assign " << GetNodeExpression(&output) << " = " << sel_name << "[" << i << " + " << expand_signal(&index, 0, -fraction(index.type)) << "];\n";
	}

	// version with foor loop - requires a bus at the output:
	// todo - this could go into the preamble part
	// f << "var logic " << width2string(width) << sel_name << "[0:" << num_in - 1 << "];\n";
	//
	//// connect input vector
	//for (int i = 0; i < num_in; ++i)
	//{
	//	auto input = *inputs[i].driver;

	//	int right = fract - fraction(input.type);
	//	int left = width - word_width(input.type) - right;
	//	assert(right >= 0);
	//	assert(left >= 0);
	//	assert(left + right + word_width(input.type) == width);

	//	f << "assign " << sel_name << "[" << i << "] = " << expand_signal(&input, left, right) << ";\n";
	//}

	//// connect outputs in a for loop
	//f << "always_comb begin :" << entity.name << "_outblock\n";
	//f << "integer i;\n";

	//f << "\tfor (i = 0; i < " << num_out << "; i = i + 1) begin\n";
	//{
	//	auto output = outputs[0];
	//	f << "\t\t" << output.name << "[i] = " << sel_name << "[i + " << expand_signal(&index, 0, -fraction(index.type)) << "];\n";
	//}
	//f << "\tend\n";
	//f << "end\n";


#elif 0
	
	int maxidx = num_in - num_out;
	assert(maxidx >= 0);
	
	// todo - this could go into the top part
	for (int i = 0; i < num_out; ++i)
	{
		auto output = outputs[i];
		f << "var logic " << width2string(width) << sel_name << "_" << i;
		if (maxidx > 0) f << "[0:" << maxidx << "];\n";
		else            f << ";\n";
	}

	// connect input vector
	for (int i = 0; i< num_out; ++i)
	{
		for (int j = 0; j <= maxidx; ++j)
		{
			auto input = *inputs[i+j].driver;

			int right = fract - fraction(input.type);
			int left = width - word_width(input.type) - right;
			assert(right >= 0);
			assert(left >= 0);
			assert(left + right + word_width(input.type) == width);

			f << "assign " << sel_name << "_" << i << "[" << j << "] = " << expand_signal(&input, left, right) << ";\n";
		}
	}

	// connect outputs
	for (int i = 0; i < num_out; ++i)
	{
		auto output = outputs[i];
		f << "assign " << output.GetName() << " = " << sel_name << "_" << i << "[" << expand_signal(&index, 0, -fraction(index.type)) << "];\n";
	}

#else
	
	int maxidx = num_in - num_out;
	assert(maxidx >= 0);
	std::string castname = entity.name + "_casted";

	// declare local signals - todo: this could go into the top part
	f << "var logic " << width2string(width) << castname << "[0:" << num_in - 1 << "];\n";
	for (int i = 0; i < width; ++i)
	{
		f << "var logic " << width2string(num_in) << sel_name << "_" << i << ";\n";
	}

	// connect input data to casted vector
	for (int i = 0; i < num_in; ++i)
	{
		auto input = *inputs[i].driver;

		int right = fract - fraction(input.type);
		int left = width - word_width(input.type) - right;
		assert(right >= 0);
		assert(left >= 0);
		assert(left + right + word_width(input.type) == width);

		f << "assign " << castname << "[" << i << "] = " << expand_signal(&input, left, right) << ";\n";
	}

	// connect casted vector to select vectors
	for (int i = 0; i < width; ++i)
	{
		f << "assign " << sel_name << "_" << i << " = ";
		for (int j = 0; j < num_in; ++j) {
			f << (j == 0 ? "{" : ", ") << castname << "[" << num_in-j-1 << "]";
			if (width > 1) f << "[" << i << "]";
		}
		f << "};\n";
	}

	// connect outputs
	for (int i = 0; i < width; ++i)
	{
		f << "assign ";
		for (int j = 0; j < num_out; ++j)
		{
			auto output = outputs[num_out-j-1];
			f << (j == 0 ? "{" : ", ") << output.name; 
			if (width > 1) f << "[" << i << "]";
		}
		f << "} = ";
		f << sel_name << "_" << i << " >> " << expand_signal(&index, 0, -fraction(index.type)) << ";\n";
	}

#endif
#endif
	f << "\n";
}

}
