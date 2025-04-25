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

    Implementation of the fixed-point Probe() block for the common
    simulator backend.

*/

#include "../global.h"

#include <oddf/design/blocks/backend/ITaggedBlock.h>

namespace dfx {
namespace backend {
namespace blocks {

class probe_block_dynfix : public BlockBase, virtual oddf::design::blocks::backend::ITaggedBlock {

private:

	std::string m_tag;
	InputPin<dynfix> input;

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t({ input.GetDrivingBlock() });
	}

	bool CanEvaluate() const override
	{
		return false;
	}

	void Evaluate() override
	{
	}

	//
	// ITaggedBlock implementation
	//

	virtual std::string GetTag() const override
	{
		return m_tag;
	}

	virtual void *GetInterface(oddf::Uid const &iid) override
	{
		if (iid == oddf::Iid<oddf::design::blocks::backend::ITaggedBlock>::value)
			return dynamic_cast<oddf::design::blocks::backend::ITaggedBlock *>(this);
		else
			return backend::BlockBase::GetInterface(iid);
	}

public:

	probe_block_dynfix(node<dynfix> const &theNode, std::string const &tag) :
		BlockBase("probe"),
		m_tag(tag),
		input(this, theNode)
	{
	}

	probe_block_dynfix(probe_block_dynfix const &) = delete;
	void operator=(probe_block_dynfix const &) = delete;
};

} // namespace blocks
} // namespace backend

namespace blocks {

void Probe(node<dynfix> const &theNode, std::string const &tag)
{
	Design::GetCurrent().NewBlock<backend::blocks::probe_block_dynfix>(theNode, tag);
}

} // namespace blocks
} // namespace dfx
