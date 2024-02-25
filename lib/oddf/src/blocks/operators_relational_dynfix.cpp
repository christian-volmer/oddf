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

	Relational operators are ==, !=, <, >, <=, >=. Implementation for
	'dynfix'.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

//
// abstract_relational_operator_block
//

template<int true1, int true2>
class relational_operator_block_dynfix : public BlockBase {

protected:

	struct Path {

		InputPin<dynfix> leftInput;
		InputPin<dynfix> rightInput;
		OutputPin<bool> output;
		int leftShift;
		int rightShift;
		bool signedCompare;

		Path(BlockBase *block, node<dynfix> const &leftNode, int leftShift, node<dynfix> const &rightNode, int rightShift, bool signedCompare) :
			leftInput(block, leftNode),
			rightInput(block, rightNode),
			output(block, false),
			leftShift(leftShift),
			rightShift(rightShift),
			signedCompare(signedCompare)
		{
			assert(leftShift >= 0);
			assert(rightShift >= 0);
			assert((leftShift == 0) || (rightShift == 0));
		}
	};

	std::list<Path> paths;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;

		for (auto &path : paths) {

			blocks.insert(path.leftInput.GetDrivingBlock());
			blocks.insert(path.rightInput.GetDrivingBlock());
		}

		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	std::string GetInputPinName(int index) const override
	{
		if (index >= 0 && index < (int)GetInputPins().size())
			return (index % 2 == 0 ? "Lhs" : "Rhs") + std::to_string(index / 2);

		assert(false);
		return "<ERROR>";
	}

	void Evaluate() override
	{
		for (auto &p : paths) {

			int result;

			if (p.leftShift > 0) {

				dynfix temp;
				p.leftInput.GetValue().CopyShiftLeft(temp, p.leftShift);

				if (p.signedCompare)
					result = -p.rightInput.GetValue().CompareSigned(temp);
				else
					result = -p.rightInput.GetValue().CompareUnsigned(temp);
			}
			else {

				dynfix temp;
				p.rightInput.GetValue().CopyShiftLeft(temp, p.rightShift);

				if (p.signedCompare)
					result = p.leftInput.GetValue().CompareSigned(temp);
				else
					result = p.leftInput.GetValue().CompareUnsigned(temp);
			}

			p.output.value = (result == true1) || (result == true2);
		}
	}

public:

	relational_operator_block_dynfix(char const *name) :
		BlockBase(name),
		paths()
	{
	}

	node<bool> add_path(node<dynfix> const &leftOperand, node<dynfix> const &rightOperand)
	{
		dynfix leftTemplate = types::DefaultFrom(leftOperand.GetDriver()->value);
		dynfix rightTemplate = types::DefaultFrom(rightOperand.GetDriver()->value);

		bool leftSigned = leftTemplate.IsSigned();
		int leftWordWidth = leftTemplate.GetWordWidth();
		int leftFraction = leftTemplate.GetFraction();
		int leftShift = 0;

		bool rightSigned = rightTemplate.IsSigned();
		int rightWordWidth = rightTemplate.GetWordWidth();
		int rightFraction = rightTemplate.GetFraction();
		int rightShift = 0;

		// Signedness
		if (leftSigned && !rightSigned) {

			rightSigned = true;
			++rightWordWidth;
		}
		else if (rightSigned && !leftSigned) {

			leftSigned = true;
			++leftWordWidth;
		}

		// Fraction
		if (leftFraction > rightFraction) {

			rightWordWidth += leftFraction - rightFraction;
			rightShift += leftFraction - rightFraction;
			rightFraction = leftFraction;
		}
		else if (rightFraction > leftFraction) {

			leftWordWidth += rightFraction - leftFraction;
			leftShift += rightFraction - leftFraction;
			leftFraction = rightFraction;
		}

		// Word width
		if (leftWordWidth > rightWordWidth)
			rightWordWidth = leftWordWidth;
		else if (rightWordWidth > leftWordWidth)
			leftWordWidth = rightWordWidth;

		assert(leftSigned == rightSigned);
		assert(leftWordWidth == rightWordWidth);
		assert(leftFraction == rightFraction);

		//dynfix commonTemplate(leftSigned, leftWordWidth, leftFraction);

		paths.emplace_back(this, leftOperand, leftShift, rightOperand, rightShift, leftSigned);
		return paths.back().output.GetNode();
	}

	bus<bool> add_bus(bus_access<dynfix> const &leftOperand, bus_access<dynfix> const &rightOperand)
	{
		int width = leftOperand.width();

		if (rightOperand.width() != width)
			throw design_error(GetFullName() + ": Operands of bus operations must have the same width.");

		bus<bool> outputBus;
		for (int i = 0; i < width; ++i)
			outputBus.append(add_path(leftOperand[i], rightOperand[i]));

		return outputBus;
	}
};

}
}

namespace blocks {


//
// Equal, NotEqual, Less, LessEqual
//

#define IMPLEMENT_RELATIONAL_FUNCTION(_name_, _name2_, _true1_, _true2_) \
	node<bool> _name_(node<dynfix> const &leftOperand, node<dynfix> const &rightOperand) \
	{ \
		return Design::GetCurrent().NewBlock<backend::blocks::relational_operator_block_dynfix<_true1_, _true2_>>(_name2_).add_path(leftOperand, rightOperand); \
	} \
	 \
	bus<bool> _name_(bus_access<dynfix> const &leftOperand, bus_access<dynfix> const &rightOperand) \
	{ \
		return Design::GetCurrent().NewBlock<backend::blocks::relational_operator_block_dynfix<_true1_, _true2_>>(_name2_).add_bus(leftOperand, rightOperand); \
	}

IMPLEMENT_RELATIONAL_FUNCTION(NotEqual, "not_equal", -1, 1)
IMPLEMENT_RELATIONAL_FUNCTION(Equal, "equal", 0, 0)
IMPLEMENT_RELATIONAL_FUNCTION(Less, "less", -1, -1)
IMPLEMENT_RELATIONAL_FUNCTION(LessEqual, "less_equal", -1, 0)

}
}
