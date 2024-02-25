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

	Relational operators are ==, !=, <, >, <=, >=. Implementation for all
	types except 'dynfix'.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

//
// abstract_relational_operator_block
//

template<typename T>
class abstract_relational_operator_block: public BlockBase {

protected:

	struct Path {

		InputPin<T> leftInput;
		InputPin<T> rightInput;
		OutputPin<bool> output;

		Path(BlockBase *block, node<T> const &leftNode, node<T> const &rightNode) :
			leftInput(block, leftNode),
			rightInput(block, rightNode),
			output(block, false)
		{
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

	abstract_relational_operator_block(char const *name) :
		BlockBase(name),
		paths()
	{
	}

public:

	node<bool> add_path(node<T> const &leftOperand, node<T> const &rightOperand)
	{
		paths.emplace_back(this, leftOperand, rightOperand);
		return paths.back().output.GetNode();
	}

	bus<bool> add_bus(bus_access<T> const &leftOperand, bus_access<T> const &rightOperand)
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

//
// equal_block, less_block, less_equal_block
//

#define MAKE_RELATIONAL_OPERATOR_BLOCK(_name_, _op_) \
	template<typename T> class _name_##_block : public abstract_relational_operator_block<T> { \
	 \
	public: \
	 \
		 _name_##_block() : abstract_relational_operator_block<T>(#_name_) {} \
	 \
	private: \
	 \
		using abstract_relational_operator_block<T>::paths; \
	 \
		void Evaluate() override \
		{ \
			for (auto &p : paths) \
				p.output.value = p.leftInput.GetValue() _op_ p.rightInput.GetValue(); \
		} \
	};

MAKE_RELATIONAL_OPERATOR_BLOCK(not_equal, !=)
MAKE_RELATIONAL_OPERATOR_BLOCK(equal, ==)
MAKE_RELATIONAL_OPERATOR_BLOCK(less, <)
MAKE_RELATIONAL_OPERATOR_BLOCK(less_equal, <=)

}
}

namespace blocks {


//
// Equal, NotEqual, Less, LessEqual
//

#define IMPLEMENT_RELATIONAL_FUNCTION(_name_, _className_, _type_) \
	node<bool> _name_(node<_type_> const &leftOperand, node<_type_> const &rightOperand) \
	{ \
		return Design::GetCurrent().NewBlock<backend::blocks::_className_<_type_>>().add_path(leftOperand, rightOperand); \
	} \
	 \
	bus<bool> _name_(bus_access<_type_> const &leftOperand, bus_access<_type_> const &rightOperand) \
	{ \
		return Design::GetCurrent().NewBlock<backend::blocks::_className_<_type_>>().add_bus(leftOperand, rightOperand); \
	}

IMPLEMENT_RELATIONAL_FUNCTION(NotEqual, not_equal_block, double)
IMPLEMENT_RELATIONAL_FUNCTION(NotEqual, not_equal_block, std::int32_t)
IMPLEMENT_RELATIONAL_FUNCTION(NotEqual, not_equal_block, std::int64_t)

IMPLEMENT_RELATIONAL_FUNCTION(Equal, equal_block, bool)
IMPLEMENT_RELATIONAL_FUNCTION(Equal, equal_block, double)
IMPLEMENT_RELATIONAL_FUNCTION(Equal, equal_block, std::int32_t)
IMPLEMENT_RELATIONAL_FUNCTION(Equal, equal_block, std::int64_t)

IMPLEMENT_RELATIONAL_FUNCTION(Less, less_block, double)
IMPLEMENT_RELATIONAL_FUNCTION(Less, less_block, std::int32_t)
IMPLEMENT_RELATIONAL_FUNCTION(Less, less_block, std::int64_t)

IMPLEMENT_RELATIONAL_FUNCTION(LessEqual, less_equal_block, double)
IMPLEMENT_RELATIONAL_FUNCTION(LessEqual, less_equal_block, std::int32_t)
IMPLEMENT_RELATIONAL_FUNCTION(LessEqual, less_equal_block, std::int64_t)

}
}
