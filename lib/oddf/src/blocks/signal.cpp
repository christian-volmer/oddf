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

    Signal() allows a normal C++ variable to provide a value to a node
    in the design.

*/

#include "../global.h"

#include <oddf/design/blocks/backend/ITaggedBlock.h>

namespace dfx {
namespace backend {
namespace blocks {

template<typename T>
class signal_block : public BlockBase, virtual oddf::design::blocks::backend::ITaggedBlock {

private:

	std::string m_tag;
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

	signal_block(T const *theVariable, std::string const &tag) :
		BlockBase("signal"),
		m_tag(tag),
		output(this, *theVariable),
		variable(theVariable)
	{
	}

	signal_block(signal_block<T> const &) = delete;
	void operator=(signal_block<T> const &) = delete;

	node<T> get_node()
	{
		return output.GetNode();
	}
};

} // namespace blocks
} // namespace backend

namespace blocks {

#define IMPLEMENT_SIGNAL_FUNCTION(_type_) \
	node<_type_> Signal(_type_ const *variable, std::string const &tag) \
	{ \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::signal_block<_type_>>(variable, tag); \
		return block.get_node(); \
	}

IMPLEMENT_SIGNAL_FUNCTION(bool)
IMPLEMENT_SIGNAL_FUNCTION(double)
IMPLEMENT_SIGNAL_FUNCTION(std::int32_t)
IMPLEMENT_SIGNAL_FUNCTION(std::int64_t)

} // namespace blocks
} // namespace dfx
