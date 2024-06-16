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

#include <oddf/utility/ContainerViews.h>

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

oddf::utility::CollectionView<oddf::design::backend::IDesignBlock> Design::GetBlockCollection() const
{
	return oddf::utility::CreateCollectionView<oddf::design::backend::IDesignBlock>(blocks);
}

} // namespace dfx
