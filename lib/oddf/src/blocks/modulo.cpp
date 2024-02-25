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

	Module() performs a modulo operation with offset.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

class modulo_block : public BlockBase {

private:

	double divisor;
	double offset;
	std::list<InputPin<double>> inputs;
	std::list<OutputPin<double>> outputs;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;
		for (auto &pin : inputs)
			blocks.insert(pin.GetDrivingBlock());
		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		auto outputFirst = outputs.begin();
		auto outputLast = outputs.end();
		auto inputFirst = inputs.begin();

		while (outputFirst != outputLast) {

			double value = (inputFirst++)->GetValue();
			(outputFirst++)->value = value - divisor * std::floor((value - offset) / divisor);
		}
	}

public:

	modulo_block(double divisor, double offset) :
		BlockBase("modulo"),
		divisor(divisor),
		offset(offset),
		inputs(),
		outputs()
	{
	}

	node<double> add_node(node<double> const &operand)
	{
		outputs.emplace_back(this, 0.0);
		inputs.emplace_back(this, operand);
		return outputs.back().GetNode();
	}

	bus<double> add_bus(bus_access<double> const &operand)
	{
		unsigned width = operand.width();

		bus<double> outputBus;
		for (unsigned i = 1; i <= width; ++i)
			outputBus.append(add_node(operand(i)));

		return outputBus;
	}
};

}
}

namespace blocks {

node<double> Modulo(node<double> const &operand, double divisor, double offset)
{
	return Design::GetCurrent().NewBlock<backend::blocks::modulo_block>(divisor, offset).add_node(operand);
}

bus<double> Modulo(bus_access<double> const &operand, double divisor, double offset)
{
	return Design::GetCurrent().NewBlock<backend::blocks::modulo_block>(divisor, offset).add_bus(operand);
}

}
}
