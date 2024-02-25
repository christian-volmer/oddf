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

	Zero() drives the value zero (or false) into a node. Terminate()
	consumes a node as input and does nothing. Disconnect() disconnects a
	node to prevent accidental use.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class terminate_block : public BlockBase {

private:

	std::list<InputPin<T>> inputs;

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t();
	}

	bool CanEvaluate() const override
	{
		return false;
	}

	void Evaluate() override
	{
	}

public:

	terminate_block() :
		BlockBase("$terminate"),
		inputs()
	{
	}

	void add_node(node<T> const &input)
	{
		inputs.emplace_back(this, input);
	}
};

}
}

namespace blocks {

#define IMPLEMENT_TERMINATE_FUNCTION(_type_) \
	void Terminate(node<_type_> const &input) \
	{ \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::terminate_block<_type_>>(); \
		block.add_node(input); \
	} \
 \
	void Terminate(bus_access<_type_> const &inputBus) \
	{ \
		unsigned width = inputBus.width(); \
 \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::terminate_block<_type_>>(); \
 \
		for (unsigned i = 1; i <= width; ++i) \
			block.add_node(inputBus(i)); \
 	}

IMPLEMENT_TERMINATE_FUNCTION(bool)
IMPLEMENT_TERMINATE_FUNCTION(double)
IMPLEMENT_TERMINATE_FUNCTION(std::int32_t)
IMPLEMENT_TERMINATE_FUNCTION(std::int64_t)
IMPLEMENT_TERMINATE_FUNCTION(dynfix)

}
}
