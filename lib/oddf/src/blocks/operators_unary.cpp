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

	Unary operators take exactly one operand. Implementation for all
	types except 'dynfix'.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

//
// abstract_unary_operator_block
//

template<typename T>
class abstract_unary_operator_block : public BlockBase {

protected:

	struct Path {

		InputPin<T> input;
		OutputPin<T> output;

		Path(BlockBase *block, node<T> const &inputNode) :
			input(block, inputNode),
			output(block, T())
		{
		}
	};

	std::list<Path> paths;

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

	abstract_unary_operator_block(char const *name) :
		BlockBase(name),
		paths()
	{
	}

public:

	node<T> add_path(node<T> const &operand)
	{
		paths.emplace_back(this, operand);
		return paths.back().output.GetNode();
	}

	bus<T> add_bus(bus_access<T> const &operand)
	{
		unsigned width = operand.width();

		bus<T> outputBus;
		for (unsigned i = 1; i <= width; ++i)
			outputBus.append(add_path(operand(i)));

		return outputBus;
	}
};

}
}


//
// negate
//

namespace backend {
namespace blocks {

template<typename T>
class negate_block : public abstract_unary_operator_block<T> {

public:

	negate_block<T>() : abstract_unary_operator_block<T>("negate") {}

private:

	using abstract_unary_operator_block<T>::paths;

	void Evaluate() override
	{
		for (auto &p : paths)
			p.output.value = -p.input.GetValue();
	}
};

}
}

namespace blocks {

#define IMPLEMENT_NEGATE_FUNCTIONS(_type_) \
	node<_type_> Negate(node<_type_> const &operand) \
	{ \
		return Design::GetCurrent().NewBlock<backend::blocks::negate_block<_type_>>().add_path(operand); \
	} \
	 \
	bus<_type_> Negate(bus_access<_type_> const &operand) \
	{ \
		return Design::GetCurrent().NewBlock<backend::blocks::negate_block<_type_>>().add_bus(operand); \
	}

IMPLEMENT_NEGATE_FUNCTIONS(double)
IMPLEMENT_NEGATE_FUNCTIONS(std::int32_t)
IMPLEMENT_NEGATE_FUNCTIONS(std::int64_t)

}


//
// not
//

namespace backend {
namespace blocks {

class not_block : public abstract_unary_operator_block<bool> {

public:

	not_block() : abstract_unary_operator_block("not") {}

private:

	void Evaluate() override
	{
		for (auto &p : paths)
			p.output.value = !p.input.GetValue();
	}
};

}
}

namespace blocks {

node<bool> Not(node<bool> const &operand)
{
	return Design::GetCurrent().NewBlock<backend::blocks::not_block>().add_path(operand);
}

bus<bool> Not(bus_access<bool> const &operand)
{
	return Design::GetCurrent().NewBlock<backend::blocks::not_block>().add_bus(operand);
}

}
}
