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

	Constant() returns a node connected to the specified constant value.

*/

#pragma once

namespace dfx {

namespace generator {

class Properties;

}

namespace backend {
namespace blocks {

template<typename T>
class constant_block : public backend::BlockBase {

private:

	std::list<backend::OutputPin<T>> outputs;

	source_blocks_t GetSourceBlocks() const override;
	bool CanEvaluate() const override;
	void Evaluate() override;

	void GetProperties(dfx::generator::Properties &properties) const override;

public:

	constant_block();
	node<T> add_output(T const &constant);
};

}
}

namespace blocks {

template<typename T>
inline node<typename types::TypeTraits<T>::internalType> Constant(T const &constant)
{
	using internalType = typename types::TypeTraits<T>::internalType;
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::constant_block<internalType>>();
	return block.add_output(constant);
}

template<typename T>
inline bus<typename types::TypeTraits<T>::internalType> RepeatedConstant(T const &constant, int length)
{
	using internalType = typename types::TypeTraits<T>::internalType;
	
	bus<internalType> outputBus;

	if (length > 0) {

		auto &block = Design::GetCurrent().NewBlock<backend::blocks::constant_block<internalType>>();
		for (int i = 0; i < length; ++i)
			outputBus.append(block.add_output(constant));
	}

	return outputBus;
}

template<typename T>
inline bus<typename types::TypeTraits<T>::internalType> Constant(std::initializer_list<T> constants)
{
	using internalType = typename types::TypeTraits<T>::internalType;

	auto &block = Design::GetCurrent().NewBlock<backend::blocks::constant_block<internalType>>();

	bus<internalType> outputBus;

	for (auto constant : constants)
		outputBus.append(block.add_output(constant));

	return outputBus;
}

template<typename T, std::size_t N>
inline bus<typename types::TypeTraits<T>::internalType> Constant(T const (&constants)[N])
{
	using internalType = typename types::TypeTraits<T>::internalType;

	auto &block = Design::GetCurrent().NewBlock<backend::blocks::constant_block<internalType>>();

	bus<internalType> outputBus;

	for (auto constant : constants)
		outputBus.append(block.add_output(constant));

	return outputBus;
}

template<typename ForwardIt>
inline bus<typename types::TypeTraits<typename std::iterator_traits<ForwardIt>::value_type>::internalType> Constant(ForwardIt first, ForwardIt last)
{
	using internalType = typename types::TypeTraits<typename std::iterator_traits<ForwardIt>::value_type>::internalType;

	auto &block = Design::GetCurrent().NewBlock<backend::blocks::constant_block<internalType>>();

	bus<internalType> outputBus;

	for (; first != last; ++first) {
		outputBus.append(block.add_output(*first));
	}

	return outputBus;
}


}
}
