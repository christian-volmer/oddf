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

    Implementation of the fixed-point Signal() block for the common
    simulator backend.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

class signal_block_dynfix : public BlockBase {

private:

	OutputPin<dynfix> output;

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
	}

public:

	signal_block_dynfix(oddf::design::NodeType const &nodeType) :
		BlockBase("signal"),
		output(this, dynfix(nodeType.IsSigned(), nodeType.GetWordWidth(), nodeType.GetFraction()))
	{
	}

	signal_block_dynfix(signal_block_dynfix const &) = delete;
	void operator=(signal_block_dynfix const &) = delete;

	node<dynfix> get_node()
	{
		return output.GetNode();
	}
};

} // namespace blocks
} // namespace backend

namespace blocks {

node<dynfix> Signal(oddf::design::NodeType const &nodeType)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::signal_block_dynfix>(nodeType);
	return block.get_node();
}

} // namespace blocks
} // namespace dfx
