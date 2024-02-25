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

	EmitSpareFlipFlops() creates the given number of spare flip-flops
	that have no connection to the design. Required to support ECO fixes
	in an ASIC design.

*/

#include "../global.h"
#include "../generator/properties.h"

namespace dfx {
namespace backend {
namespace blocks {

class spare_block : public BlockBase {

private:

	int mNumberOfBits;

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

	bool CanRemove() const override
	{
		return false;
	}

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		properties.SetInt("NumberOfBits", mNumberOfBits);
	}

public:

	spare_block(int numberOfBits) :
		BlockBase("spare_ff"),
		mNumberOfBits(numberOfBits)
	{
	}
};

}
}

namespace blocks {

void EmitSpareFlipFlops(int numberOfBits)
{
	Design::GetCurrent().NewBlock<backend::blocks::spare_block>(numberOfBits);
}

}
}
