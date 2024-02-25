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

	Verilog code emission for the Recorder block.

*/

#include "../global.h"
#include "entities.h"

namespace entities {

void Recorder::WriteCode(std::ofstream &f, dfx::generator::Instance &, dfx::generator::Entity &entity) const
{
	std::string nrstName = exporter->GetConfiguration().negatedResetPinName;

	f << "// " << entity.name << " - playback module\n";

	int stimwidth = entity.properties.GetInt("StimulusWidth");
	int refwidth  = entity.properties.GetInt("ReferenceWidth");

	f << "integer TB_f_stim, TB_f_ref; // will be forced from outside!\n";
	f << "integer TB_line_count, TB_error_count, TB_good_count;\n";
	f << "integer TB_num_read_stim;\n";
	f << "integer TB_num_read_ref;\n";
	f << "var logic TB_enable;\n";
	f << "var logic [" << stimwidth - 1 << ":" << 0 << "] TB_current_stimulus_bits;\n";
	f << "var logic [" << refwidth - 1 << ":" << 0 << "] TB_current_reference_bits;\n";
	f << "always @(posedge clk or negedge " << nrstName << ")\n";
	f << "begin\n";
	f << "\tif (~" << nrstName << ") begin\n";
	f << "\t\tTB_line_count <= 0;\n";
	f << "\t\tTB_error_count <= 0;\n";
	f << "\t\tTB_good_count <= 0;\n";
	f << "\t\tTB_num_read_stim <= 0;\n";
	f << "\t\tTB_num_read_ref <= 0;\n";
	f << "\t\tTB_current_stimulus_bits <= " << stimwidth << "'d0;\n";
	f << "\t\tTB_current_reference_bits <= " << refwidth << "'d0;\n";
	f << "\t\tTB_enable <= 1'b0;\n";
	f << "\tend\n";
	f << "\telse begin\n";
	
	std::string format = "";
	std::string params = "";
	for (int i = MaxTestDataChunk * (stimwidth / MaxTestDataChunk); i >= 0; i -= MaxTestDataChunk) {
		int upr = i + MaxTestDataChunk - 1;
		if (upr >= stimwidth) upr = stimwidth - 1;
		format.append("%" + std::to_string((upr-i+4)/4) + "x");
		params.append(", TB_current_stimulus_bits[" + std::to_string(upr) + ((upr != i) ? ":" + std::to_string(i) + "]" : ""));
	}
	f << "\t\tTB_num_read_stim = $fscanf(TB_f_stim, \"" << format << "\"" << params << ");\n";
	
	format = "";
	params = "";
	for (int i = MaxTestDataChunk * (refwidth / MaxTestDataChunk); i >= 0; i -= MaxTestDataChunk) {
		int upr = i + MaxTestDataChunk - 1;
		if (upr >= refwidth) upr = refwidth - 1;
		format.append("%" + std::to_string((upr - i + 4) / 4) + "x");
		params.append(", TB_current_reference_bits[" + std::to_string(upr) + ((upr != i) ? ":" + std::to_string(i) + "]" : ""));
	}
	f << "\t\tTB_num_read_ref = $fscanf(TB_f_ref, \"" << format << "\"" << params << ");\n";

	f << "\t\tTB_enable <= 1'b1;\n";
	f << "\t\tTB_line_count <= TB_line_count+1;\n";
	f << "\t\tif ($feof(TB_f_stim) || $feof(TB_f_ref)) begin\n";
	f << "\t\t\tTB_enable <= 1'b0;\n";
	f << "\t\tend\n";
	f << "\t\telse begin\n";
	f << "\t\t\tTB_enable <= 1'b1;\n";
	f << "\t\tend\n";
	f << "\tend\n";
	f << "end\n";

	f << "\n";
}

}
