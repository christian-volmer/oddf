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
	number of operands. Implementation of the Times() operation for
	dynfix.

*/

#include "../global.h"
#include "../generator/properties.h"

namespace dfx {
namespace backend {
namespace blocks {

//
// times_operator_block_dynfix
//

class times_operator_block_dynfix : public BlockBase {

private:

	struct Factor {

		InputPin<dynfix> input;

		Factor(BlockBase *block, node<dynfix> const &inputNode) :
			input(block, inputNode)
		{
		}
	};

	struct Product {

		std::list<Factor> factors;
		OutputPin<dynfix> output;

		Product(BlockBase *block, dynfix const &outputTemplate) :
			factors(),
			output(block, outputTemplate)
		{
		}
	};


	std::list<Product> products;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;
		for (auto &product : products) {

			for (auto &factor : product.factors)
				blocks.insert(factor.input.GetDrivingBlock());
		}
		return blocks;
	}

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		properties.SetInt("NumberOfFactors", (int)products.front().factors.size());
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		for (auto &product : products) {

			auto factorIt = product.factors.begin();
			dynfix factor1 = factorIt->input.GetValue();
			dynfix factor2 = std::next(factorIt)->input.GetValue();

			factor1.CopyMultiplyUnsigned(product.output.value, (std::uint32_t)factor2.data[0]);
			for (int j = 1; j < dynfix::MAX_FIELDS - 1; ++j)
				factor1.AccumulateMultiplyUnsigned(product.output.value, factor2.data[j], j);

			if (factor2.IsSigned())
				factor1.AccumulateMultiplySigned(product.output.value, factor2.data[dynfix::MAX_FIELDS - 1], dynfix::MAX_FIELDS - 1);
			else
				factor1.AccumulateMultiplyUnsigned(product.output.value, factor2.data[dynfix::MAX_FIELDS - 1], dynfix::MAX_FIELDS - 1);
		}
	}

public:

	times_operator_block_dynfix() :
		BlockBase("times"),
		products()
	{
	}

	template<typename InputIt> node<dynfix> add_path(InputIt first, InputIt last)
	{
		int numberOfFactors = (int)std::distance(first, last);

		if (numberOfFactors != 2)
			throw design_error(GetFullName() + ": multiplication requires exactly two factors.");

		int wordWidth = 0;
		bool isSigned = false;
		int fraction = 0;

		for (auto factorIt = first; factorIt != last; ++factorIt) {

			bool thisSigned = (*factorIt)->GetType().IsSigned();
			int thisFraction = (*factorIt)->GetType().GetFraction();
			int thisWordWidth = (*factorIt)->GetType().GetWordWidth();

			assert(thisWordWidth > 0);

			isSigned |= thisSigned;
			fraction += thisFraction;

			// when an operand is ufix<1, ?> this corresponds to a shift of the binary point and the word length does not increase.
			if (thisSigned || thisWordWidth > 1)
				wordWidth += thisWordWidth;
		}

		// All operands were ufix<1, ?>. So result should have one bit as well.
		if (wordWidth == 0)
			wordWidth = 1;

		products.emplace_back(this, dynfix(isSigned, wordWidth, fraction));

		for (auto input = first; input != last; ++input) {

			node<dynfix> const &factorNode = **input;
			products.back().factors.emplace_back(this, factorNode);
			++first;
		}

		return products.back().output.GetNode();
	}
};

}
}

namespace blocks {


//
// times
//

node<dynfix> Times(node<dynfix> const &op1, node<dynfix> const &op2) 
{ 
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::times_operator_block_dynfix>();
	std::initializer_list<node<dynfix> const *> operands = { &op1, &op2 };
	return block.add_path(operands.begin(), operands.end());
}
	
bus<dynfix> Times(bus_access<dynfix> const &op1, bus_access<dynfix> const &op2) 
{ 
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::times_operator_block_dynfix>();

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
	
}
}
