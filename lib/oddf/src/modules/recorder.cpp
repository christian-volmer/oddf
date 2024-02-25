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

	The recorder can be used to record the inputs and outputs of a design
	during simulation. Recorded content can be written to file and used
	in a Verilog simulation to confirm the correctness of the generated
	code.

*/

#include "../global.h"
#include "../generator/properties.h"
#include "recorder.h"

namespace dfx {
namespace backend {
namespace blocks {


recorder_block::recorder_block(dfx::modules::Recorder *Recorder) :
	BlockBase("recorder"),
	Recorder(Recorder),
	StimWidth(0),
	RefWidth(0)
{ }

void recorder_block::GetProperties(dfx::generator::Properties &properties) const
{
	properties.SetInt("StimulusWidth", Recorder->GetStimWidth());
	properties.SetInt("ReferenceWidth", Recorder->GetRefWidth());
}

std::string stimcheck_block_base::valueToString(bool val) const
{
	return val ? "1" : "0";
};

std::string stimcheck_block_base::valueToString(const dynfix &val) const
{
	std::string result;
	result.reserve(val.GetWordWidth() + 1);
	for (int i = val.GetWordWidth() - 1; i >= 0; --i)
	{
		result.append((val.data[i / 32] & (1 << (i % 32))) ? "1" : "0");
	}
	return result;
};

int stimcheck_block_base::getNodeWidth(const node<bool> &) const
{
	return 1;
};

int stimcheck_block_base::getNodeWidth(const node<dynfix> &val) const
{
	return val.GetType().GetWordWidth();
};

int stimcheck_block_base::getBitWidth() const 
{ 
	return bitWidth;
};
int stimcheck_block_base::getStartIndex() const
{ 
	return startIndex; 
};

void stimcheck_block_base::GetPropertiesBase(dfx::generator::Properties &properties) const
{
	properties.SetInt("BitWidth", bitWidth);
	properties.SetInt("StartIndex", startIndex);
}

}
}


namespace modules {


#define IMPLEMENT_CONSTRUCT_ADD(BN,TY,LN,IN,IX) \
	int bitWidth = ((IN).GetType().IsClass(types::TypeDescription::Boolean) ? 1 : (IN).GetType().GetWordWidth()); \
	auto block = &dfx::Design::GetCurrent().NewBlock<dfx::backend::blocks::BN<TY>>((IN), IX); \
	LN.push_back((dfx::backend::blocks::stimcheck_block_base *)block); \
	IX += bitWidth; \

#define IMPLEMENT_ADDSTIM_NODE(TY) \
node<TY> Recorder::AddStimulus(node<TY> const &in) \
{ \
	IMPLEMENT_CONSTRUCT_ADD(stim_block,TY,stims,in,stim_index); \
	return block->getOutNode(); \
}

#define IMPLEMENT_ADDSTIM_BUS(TY) \
bus<TY> Recorder::AddStimulus(bus<TY> const &in_bus)\
{\
	bus<TY> out_bus;\
	for (int i = 0; i < in_bus.width(); ++i)\
		{\
		IMPLEMENT_CONSTRUCT_ADD(stim_block,TY,stims,in_bus[i],stim_index); \
		out_bus.append(block->getOutNode());\
		}\
	return out_bus;\
}

#define IMPLEMENT_ADDCHECK_NODE(TY) \
node<TY> Recorder::AddChecker(node<TY> const &in)\
{\
	IMPLEMENT_CONSTRUCT_ADD(checker_block,TY,checkers,in,check_index); \
	return in; \
}

#define IMPLEMENT_ADDCHECK_BUS(TY) \
bus<TY> Recorder::AddChecker(bus<TY> const &in_bus)\
{\
	for (int i = 0; i < in_bus.width(); ++i) \
	{\
		IMPLEMENT_CONSTRUCT_ADD(checker_block,TY,checkers,in_bus[i],check_index); \
	}\
	return in_bus; \
}

IMPLEMENT_ADDSTIM_NODE(bool);
IMPLEMENT_ADDSTIM_BUS (bool);
IMPLEMENT_ADDSTIM_NODE(dynfix);
IMPLEMENT_ADDSTIM_BUS (dynfix);
IMPLEMENT_ADDCHECK_NODE(bool);
IMPLEMENT_ADDCHECK_BUS (bool);
IMPLEMENT_ADDCHECK_NODE(dynfix);
IMPLEMENT_ADDCHECK_BUS (dynfix);


Recorder::Recorder() :
stim_index(0),
check_index(0)
{
	// instantiate a recorder block
	rec_block = &dfx::Design::GetCurrent().NewBlock<dfx::backend::blocks::recorder_block>(this);
}


std::string Recorder::Binary2Hex(std::string &bin)
{
	std::string hex;
	for (int i = (int)bin.size() - 1; i >= 0; i -= 4) {

		int val = 0;

		for (int j = 0; j < 4; j++)
		{
			if (j <= i) {
				if (bin[i - j] == '1') val |= (1 << j);
			}
		}
		hex.insert(0, string_printf("%1x", val));
	}
	return hex;
}


void Recorder::internal_WriteToFile(const std::string &fileName, std::list<dfx::backend::blocks::stimcheck_block_base *> &list)
{
	std::ofstream f(fileName);
	if (f.is_open()) 
	{
		auto num_lines = stims.front()->getNumElements();
				
		for (int l = 0; l < num_lines; ++l)
		{
			std::string line;
			line.clear();

			for (dfx::backend::blocks::stimcheck_block_base * s : list) 
			{
				if (l==0) assert(num_lines == s->getNumElements());

				line = s->getElement(l) + line;
			}
			f << Binary2Hex(line) << "\n";
		}
	}
	else
		throw std::ofstream::failure("Cannot open file 'fileName'");

}

void Recorder::WriteToFile(const std::string &basePath)
{
	internal_WriteToFile(basePath + "\\stimulus.txt", stims);
	internal_WriteToFile(basePath + "\\reference.txt", checkers);
}

int Recorder::GetStimWidth()
{
	return stim_index;
}

int Recorder::GetRefWidth()
{
	return check_index;
}

void Recorder::Clear()
{
	for (dfx::backend::blocks::stimcheck_block_base * s : stims) s->clear();
	for (dfx::backend::blocks::stimcheck_block_base * c : checkers) c->clear();
}

}
}
