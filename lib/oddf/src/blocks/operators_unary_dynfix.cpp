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

	Unary operators take exactly one operand. Implementation for
	'dynfix'.

*/

#include "../global.h"

namespace dfx {

//
// negate (dynfix)
//

namespace backend {
namespace blocks {

class negate_block_dynfix : public BlockBase {

private:

	struct path {

		InputPin<dynfix> input;
		OutputPin<dynfix> output;

		path(BlockBase *block, node<dynfix> const &inputNode, dynfix const &outputTemplate) :
			input(block, inputNode),
			output(block, outputTemplate)
		{
		}
	};

	std::list<path> paths;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;

		for (auto &path : paths)
			blocks.insert(path.input.GetDrivingBlock());

		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		for (auto &p : paths)
			p.input.GetValue().CopyNegate(p.output.value);
	}

public:

	negate_block_dynfix() :
		BlockBase("negate"),
		paths()
	{
	}

	node<dynfix> add_node(node<dynfix> const &operand)
	{
		dynfix operandTemplate = types::DefaultFrom(operand.GetDriver()->value);
		dynfix outputTemplate(true, operandTemplate.GetWordWidth() + 1, operandTemplate.GetFraction());

		paths.emplace_back(this, operand, outputTemplate);
		return paths.back().output.GetNode();
	}

	bus<dynfix> add_bus(bus_access<dynfix> const &operand)
	{
		unsigned width = operand.width();

		bus<dynfix> outputBus;
		for (unsigned i = 1; i <= width; ++i)
			outputBus.append(add_node(operand(i)));

		return outputBus;
	}
};

}
}

namespace blocks {

node<dynfix> Negate(node<dynfix> const &operand)
{
	return Design::GetCurrent().NewBlock<backend::blocks::negate_block_dynfix>().add_node(operand);
}

bus<dynfix> Negate(bus_access<dynfix> const &operand)
{
	return Design::GetCurrent().NewBlock<backend::blocks::negate_block_dynfix>().add_bus(operand);
}

}

}
