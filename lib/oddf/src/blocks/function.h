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

	Function() allows the insertion of a normal C++ function into the
	design.

*/

#pragma once

namespace dfx {
namespace backend {
namespace blocks {

//
// function_block
//

template<typename outT, typename inT>
class function_block : public BlockBase {

private:

	std::function<outT(inT const &)> function;

	std::list<OutputPin<outT>> outputs;
	std::list<InputPin<inT>> inputs;

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

		while (outputFirst != outputLast)
			(outputFirst++)->value = function((inputFirst++)->GetValue());
	}

public:

	function_block(std::function<outT(inT const &)> const &function) : 
		BlockBase("function"), 
		function(function), 
		outputs(), 
		inputs()
	{
	}

	node<outT> add_node(node<inT> const &operand)
	{
		outputs.emplace_back(this, outT());
		inputs.emplace_back(this, operand);
		return outputs.back().GetNode();
	}

	bus<outT> add_bus(bus_access<inT> const &operand)
	{
		unsigned width = operand.width();

		bus<outT> outputBus;
		for (unsigned i = 1; i <= width; ++i)
			outputBus.append(add_node(operand(i)));

		return outputBus;
	}
};

}
}

namespace blocks {

template<typename inT, typename functionT>
auto inline Function(node<inT> const &operand, functionT function) -> node<decltype(function(std::declval<inT>()))>
{
	return Design::GetCurrent().NewBlock<backend::blocks::function_block<decltype(function(std::declval<inT>())), inT>>(function).add_node(operand);
}

template<typename inT, typename functionT>
auto inline Function(bus_access<inT> const &operand, functionT function) -> bus<decltype(function(std::declval<inT>()))>
{
	return Design::GetCurrent().NewBlock<backend::blocks::function_block<decltype(function(std::declval<inT>())), inT>>(function).add_bus(operand);
}

}
}
