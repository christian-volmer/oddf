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

	Flat operators are operators that can in principle be extended to any
	number of operands. Implementation for all types except 'dynfix'.

*/

#include "../global.h"
#include "../generator/properties.h"

namespace dfx {
namespace backend {
namespace blocks {

//
// abstract_flat_operator_block
//

template<typename T>
class abstract_flat_operator_block: public BlockBase {

protected:

	std::list<OutputPin<T>> outputs;
	std::list<InputPin<T>> inputs;

	T DefaultValue;
	unsigned NumberOfOperands;

	abstract_flat_operator_block(char const *name, T const &defaultValue) :
		BlockBase(name),
		outputs(),
		inputs(),
		DefaultValue(defaultValue),
		NumberOfOperands(0)
	{
	}

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;
		for (auto &pin : inputs)
			blocks.insert(pin.GetDrivingBlock());
		return blocks;
	}

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		properties.SetInt("NumberOfOperands", NumberOfOperands);
	}

	std::string GetOutputPinDescription(int index, int &groupIndex, int &busSize, int &busIndex) const override
	{
		/*

		Uncomment for bus outputs.

		int length = (int)outputs.size();

		if (length > 1 && index >= 0 && index < length) {

			groupIndex = 0;
			busSize = length;
			busIndex = index;
			return "Out";
		}
		*/

		return BlockBase::GetOutputPinDescription(index, groupIndex, busSize, busIndex);
	}

public:

	template<typename InputIt> node<T> add_path(InputIt first, InputIt last)
	{
		NumberOfOperands = 0;

		outputs.emplace_back(this, T());

		while (first != last) {

			inputs.emplace_back(this, **(first++));
			++NumberOfOperands;
		}

		return outputs.back().GetNode();
	}
};

}
}


//
// or_block, and_block, xor_block
//

namespace backend {
namespace blocks {

#define MAKE_FLAT_OPERATOR_BLOCK(_name_, _op_, _default_) \
	template<typename T> class _name_##_block : public abstract_flat_operator_block<T> { \
	 \
	public: \
	 \
		 _name_##_block() : abstract_flat_operator_block<T>(#_name_, T(_default_)) {} \
	 \
	private: \
	 \
		using abstract_flat_operator_block<T>::outputs; \
		using abstract_flat_operator_block<T>::inputs; \
		using abstract_flat_operator_block<T>::DefaultValue; \
		using abstract_flat_operator_block<T>::NumberOfOperands; \
	 \
		bool CanEvaluate() const override \
		{ \
			return true; \
		} \
	 \
		void Evaluate() override \
		{ \
			auto outputIt = outputs.begin(); \
			auto outputEnd = outputs.end(); \
			auto inputIt = inputs.begin(); \
	 \
	 	 	while (outputIt != outputEnd) { \
	 \
				T result = DefaultValue; \
	 \
				for (unsigned i = 0; i < NumberOfOperands; ++i) { \
	 \
					T value = (inputIt++)->GetValue(); \
					result = result _op_ value; \
				} \
	 \
				(outputIt++)->value = result; \
			} \
		} \
	};

MAKE_FLAT_OPERATOR_BLOCK(or, ||, false)
MAKE_FLAT_OPERATOR_BLOCK(and, &&, true)
MAKE_FLAT_OPERATOR_BLOCK(xor, !=, false)

MAKE_FLAT_OPERATOR_BLOCK(plus, +, 0)
MAKE_FLAT_OPERATOR_BLOCK(times, *, 1)

template<typename blockT, typename T>
static inline node<T> FlatOperator(node<T> const &op1, node<T> const &op2)
{
	auto &block = Design::GetCurrent().NewBlock<blockT>();
	std::initializer_list<node<T> const *> operands = { &op1, &op2 };
	return block.add_path(operands.begin(), operands.end());
}

template<typename blockT, typename T>
static inline bus<T> FlatOperator(bus_access<T> const &op1, bus_access<T> const &op2)
{
	auto &block = Design::GetCurrent().NewBlock<blockT>();

	int width = op1.width();
	if (op2.width() != width)
		throw design_error(block.GetFullName() + ": Operands of bus operations must have the same width.");

	bus<T> outputBus;
	for (int i = 1; i <= width; ++i) {

		std::initializer_list<node<T> const *> operands = { &op1(i), &op2(i) };
		outputBus.append(block.add_path(operands.begin(), operands.end()));
	}

	return outputBus;
}

template<typename blockT, typename T>
static inline node<T> FlatReductionOperator(bus_access<T> const &operand)
{
	// TODO: implement iterators for busses
	std::vector<node<T> const *> busNodes;

	for (int i = 1; i <= operand.width(); ++i)
		busNodes.push_back(&operand(i));

	auto &block = Design::GetCurrent().NewBlock<blockT>();
	return block.add_path(busNodes.begin(), busNodes.end());
}

}
}

namespace blocks {


//
// or, and, xor
//

#define IMPLEMENT_BOOLEAN_FUNCTIONS(_name_, _className_) \
	node<bool> _name_(node<bool> const &op1, node<bool> const &op2) \
	{ \
		return backend::blocks::FlatOperator<backend::blocks::_className_<bool>>(op1, op2); \
	} \
	 \
	bus<bool> _name_(bus_access<bool> const &op1, bus_access<bool> const &op2) \
	{ \
		return backend::blocks::FlatOperator<backend::blocks::_className_<bool>>(op1, op2); \
	} \
	 \
	node<bool> Reduction##_name_(bus_access<bool> const &operand) \
	{ \
		return backend::blocks::FlatReductionOperator<backend::blocks::_className_<bool>>(operand); \
	}

IMPLEMENT_BOOLEAN_FUNCTIONS(Or, or_block)
IMPLEMENT_BOOLEAN_FUNCTIONS(And, and_block)
IMPLEMENT_BOOLEAN_FUNCTIONS(Xor, xor_block)


//
// plus
//

#define IMPLEMENT_PLUS_FUNCTIONS(_type_) \
	node<_type_> Plus(node<_type_> const &op1, node<_type_> const &op2) \
	{ \
		return backend::blocks::FlatOperator<backend::blocks::plus_block<_type_>>(op1, op2); \
	} \
	 \
	bus<_type_> Plus(bus_access<_type_> const &op1, bus_access<_type_> const &op2) \
	{ \
		return backend::blocks::FlatOperator<backend::blocks::plus_block<_type_>>(op1, op2); \
	} \
	 \
	node<_type_> Sum(bus_access<_type_> const &operand) \
	{ \
		return backend::blocks::FlatReductionOperator<backend::blocks::plus_block<_type_>>(operand); \
	}


IMPLEMENT_PLUS_FUNCTIONS(double)
IMPLEMENT_PLUS_FUNCTIONS(std::int32_t)
IMPLEMENT_PLUS_FUNCTIONS(std::int64_t)


//
// times
//

#define IMPLEMENT_TIMES_FUNCTIONS(_type_) \
	node<_type_> Times(node<_type_> const &op1, node<_type_> const &op2) \
	{ \
		return backend::blocks::FlatOperator<backend::blocks::times_block<_type_>>(op1, op2); \
	} \
	 \
	bus<_type_> Times(bus_access<_type_> const &op1, bus_access<_type_> const &op2) \
	{ \
		return backend::blocks::FlatOperator<backend::blocks::times_block<_type_>>(op1, op2); \
	}


IMPLEMENT_TIMES_FUNCTIONS(double)
IMPLEMENT_TIMES_FUNCTIONS(std::int32_t)
IMPLEMENT_TIMES_FUNCTIONS(std::int64_t)

}
}
