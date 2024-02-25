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

	Signal() allows a normal C++ variable to provide a value to a node in
	the design.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class signal_block : public BlockBase {

private:

	OutputPin<T> output;
	T const *variable;

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t();
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		output.value = *variable;
		SetDirty();
	}

public:

	signal_block(T const *theVariable) :
		BlockBase("signal"),
		output(this, *theVariable),
		variable(theVariable)
	{
	}

	signal_block(signal_block<T> const &) = delete;
	signal_block &operator =(signal_block<T> const &) = delete;

	node<T> get_node()
	{
		return output.GetNode();
	}
};

}
}

namespace blocks {

#define IMPLEMENT_SIGNAL_FUNCTION(_type_) \
	node<_type_> Signal(_type_ const *variable) \
	{ \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::signal_block<_type_>>(variable); \
		return block.get_node(); \
	} 

IMPLEMENT_SIGNAL_FUNCTION(bool)
IMPLEMENT_SIGNAL_FUNCTION(double)
IMPLEMENT_SIGNAL_FUNCTION(std::int32_t)
IMPLEMENT_SIGNAL_FUNCTION(std::int64_t)

}
}
