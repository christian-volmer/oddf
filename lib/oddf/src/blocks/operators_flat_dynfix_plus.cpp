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

	Flat operators are operators that can in principle be extended to any
	number of operands. Implementation of the Plus() operation for
	dynfix.

*/

#include "../global.h"
#include "../generator/properties.h"

namespace dfx {
namespace backend {
namespace blocks {


//
// plus_operator_block_dynfix
//

class plus_operator_block_dynfix : public BlockBase {

private:

	struct Summand {

		InputPin<dynfix> input;
		int align;

		Summand(BlockBase *block, int align, node<dynfix> const &inputNode) :
			input(block, inputNode),
			align(align)
		{
			assert(align >= 0);
		}
	};

	struct Sum {

		std::list<Summand> summands;
		OutputPin<dynfix> output;

		Sum(BlockBase *block, dynfix const &outputTemplate) :
			summands(),
			output(block, outputTemplate)
		{
		}
	};


	std::list<Sum> sums;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;
		for (auto &sum : sums) {

			for (auto &summand : sum.summands)
				blocks.insert(summand.input.GetDrivingBlock());
		}
		return blocks;
	}

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		properties.SetInt("NumberOfSummands", (int)sums.front().summands.size());
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		for (auto &sum : sums) {

			auto summandIt = sum.summands.begin();
			dynfix value = (*summandIt).input.GetValue();
			value.CopyShiftLeft(sum.output.value, (*summandIt).align);
			++summandIt;

			for (; summandIt != sum.summands.end(); ++summandIt) {

				value = (*summandIt).input.GetValue();
				value.AccumulateShiftLeft(sum.output.value, (*summandIt).align);
			}
		}
	}

public:

	plus_operator_block_dynfix() :
		BlockBase("plus"),
		sums()
	{
	}

	template<typename InputIt> node<dynfix> add_path(InputIt first, InputIt last)
	{
		int numberOfSummands = (int)std::distance(first, last);
		assert(sums.empty() || (numberOfSummands == (int)sums.front().summands.size()));

		std::vector<dynfix> inputTypes;
		inputTypes.reserve(numberOfSummands);
		std::transform(first, last, std::back_inserter(inputTypes), [](node<dynfix> const *input) { return input->GetDriver()->value; });

		dynfix commonTemplate = dynfix::CommonRepresentation(inputTypes.cbegin(), inputTypes.cend());

		bool isSigned = commonTemplate.IsSigned();
		int wordWidth = commonTemplate.GetWordWidth();
		int fraction = commonTemplate.GetFraction();

		// Add ceil(log2(NumberOfSummands)) to obtain the final word width. 
		// TODO: this procedure sometimes overestimates the required number of bits. Find a better approach.
		wordWidth += (int)std::ceil(std::log2(numberOfSummands));
		sums.emplace_back(this, dynfix(isSigned, wordWidth, fraction));

		for (auto input = first; input != last; ++input) {

			node<dynfix> const &summandNode = **input;
			sums.back().summands.emplace_back(this, fraction - summandNode.GetDriver()->value.GetFraction(), summandNode);
		}

		return sums.back().output.GetNode();
	}
};

}
}

namespace blocks {


//
// plus
//

node<dynfix> Plus(node<dynfix> const &op1, node<dynfix> const &op2) 
{ 
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::plus_operator_block_dynfix>();
	std::initializer_list<node<dynfix> const *> operands = { &op1, &op2 };
	return block.add_path(operands.begin(), operands.end());
}
	
bus<dynfix> Plus(bus_access<dynfix> const &op1, bus_access<dynfix> const &op2) 
{ 
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::plus_operator_block_dynfix>();

	int width = op1.width();
	if (op2.width() != width)
		throw design_error(block.GetFullName() + ": Operands of bus operations must have the same width.");

	bus<dynfix> outputBus;
	for (int i = 1; i <= width; ++i) {

		std::initializer_list<node<dynfix> const *> operands = { &op1(i), &op2(i) };
		outputBus.append(block.add_path(operands.begin(), operands.end()));
	}

	return outputBus;
}
	
node<dynfix> Sum(bus_access<dynfix> const &operand) 
{ 
	// TODO: implement iterators for busses
	std::vector<node<dynfix> const *> busNodes;

	for (int i = 1; i <= operand.width(); ++i)
		busNodes.push_back(&operand(i));

	auto &block = Design::GetCurrent().NewBlock<backend::blocks::plus_operator_block_dynfix>();
	return block.add_path(busNodes.begin(), busNodes.end());
}

}
}
