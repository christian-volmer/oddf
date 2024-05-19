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

    Implementation of the 'Design' class, which manages the design
    hierarchy and the collection of design blocks.

*/

#include "global.h"

#include "node.h"
#include "messages.h"
#include "hierarchy.h"

#include "blocks/constant.h"

namespace dfx {

using backend::BlockBase;

static Design *g_pCurrentDesign = nullptr;

//
// design
//

Design::Design() :
	hierarchy(std::unique_ptr<dfx::Hierarchy>(new dfx::Hierarchy(this))),
	blocks(),
	hasCustomDefaultEnable(false),
	customDefaultEnable(nullptr)
{
	g_pCurrentDesign = this;
}

Design &Design::GetCurrent()
{
	if (!g_pCurrentDesign)
		throw design_error("There is no current design. Create a design using the design class.");

	return *g_pCurrentDesign;
}

Hierarchy &Design::GetHierarchy() const
{
	return *hierarchy;
}

std::vector<std::unique_ptr<backend::BlockBase>> const &Design::GetBlocks() const
{
	return blocks;
}

void Design::Report(std::basic_ostream<char> &os) const
{
	using std::endl;
	using std::setw;

	// hierarchy->report(os);

	std::unordered_map<std::string, int> blockNames;
	for (auto &block : blocks)
		blockNames[block->GetClassName()]++;

	os << " --- Blocks --- " << endl
	   << endl;

	os << " Total number of blocks : " << blocks.size() << endl;
	os << endl;
	os << " Counts by block type" << endl;
	os << endl;

	std::size_t maximumWidth = 1;
	for (auto const &x : blockNames)
		maximumWidth = std::max(maximumWidth, x.first.length());

	for (auto const &x : blockNames)
		os << setw(maximumWidth + 3) << x.first << " : " << x.second << endl;

	os << endl;
}

std::unique_ptr<oddf::utility::IConstEnumerator<oddf::design::backend::IDesignBlock>> Design::GetBlockEnumerator() const
{
	class BlockEnumerator : public oddf::utility::IConstEnumerator<oddf::design::backend::IDesignBlock> {

	private:

		std::vector<std::unique_ptr<dfx::backend::BlockBase>> const &m_Blocks;
		size_t m_Position;

	public:

		BlockEnumerator(dfx::Design const &design) :
			m_Blocks(design.GetBlocks()),
			m_Position(size_t(-1))
		{
		}

		virtual oddf::design::backend::IDesignBlock const &GetCurrent() const
		{
			if (m_Position < m_Blocks.size())
				return *m_Blocks[m_Position].get();
			else
				throw std::out_of_range("BlockEnumerator::GetCurrent(): there is no current element.");
		}

		virtual bool MoveNext() noexcept
		{
			if (m_Position == size_t(-1) || m_Position < m_Blocks.size())
				++m_Position;

			return m_Position < m_Blocks.size();
		}

		virtual void Reset() noexcept
		{
			m_Position = size_t(-1);
		}
	};

	return std::unique_ptr<oddf::utility::IConstEnumerator<oddf::design::backend::IDesignBlock>> { new BlockEnumerator(*this) };
}

} // namespace dfx
