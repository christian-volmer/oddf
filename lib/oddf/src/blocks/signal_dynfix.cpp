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

#include <oddf/design/blocks/backend/ITaggedBlock.h>

namespace dfx {
namespace backend {
namespace blocks {

class signal_block_dynfix : public BlockBase, virtual oddf::design::blocks::backend::ITaggedBlock {

private:

	std::string m_tag;
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

	signal_block_dynfix(oddf::design::NodeType const &nodeType, std::string const &tag) :
		BlockBase("signal"),
		m_tag(tag),
		output(this, dynfix(nodeType.IsSigned(), nodeType.GetWordWidth(), nodeType.GetFraction()))
	{
		if (!(tag.empty() || oddf::ResourcePath::IsValidElement(tag)))
			throw oddf::Exception(oddf::ExceptionCode::InvalidArgument, "The tag contains characters that are not allowed as part of a resource path element.");
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

node<dynfix> Signal(oddf::design::NodeType const &nodeType, std::string const &tag)
{
	assert(nodeType.GetTypeId() == oddf::design::NodeType::FIXED_POINT);
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::signal_block_dynfix>(nodeType, tag);
	return block.get_node();
}

} // namespace blocks
} // namespace dfx
