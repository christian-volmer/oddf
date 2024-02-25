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

	Probe() allows a normal C++ variable to probe the value of the
	provided node during simulation.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class probe_block : public BlockBase {

private:

	InputPin<T> input;
	T variable;

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t({ input.GetDrivingBlock() });
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		variable = input.GetValue();
	}

public:

	probe_block(node<T> const &theNode) :
		BlockBase("probe"),
		input(this, theNode),
		variable()
	{
	}

	probe_block(probe_block<T> const &) = delete;
	probe_block &operator =(probe_block<T> const &) = delete;

	T const *get_pointer()
	{
		return &variable;
	}
};

}
}

namespace blocks {

#define IMPLEMENT_PROBE_FUNCTION(_type_) \
	 _type_ const *Probe(node<_type_> const &theNode) \
	{ \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::probe_block<_type_>>(theNode); \
		return block.get_pointer(); \
	} 

IMPLEMENT_PROBE_FUNCTION(bool)
IMPLEMENT_PROBE_FUNCTION(double)
IMPLEMENT_PROBE_FUNCTION(std::int32_t)
IMPLEMENT_PROBE_FUNCTION(std::int64_t)

}
}
