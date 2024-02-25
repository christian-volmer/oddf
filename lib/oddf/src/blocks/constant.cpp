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

#include "../global.h"
#include "../generator/properties.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T>
constant_block<T>::constant_block() :
BlockBase("constant"),
outputs()
{
}

template<typename T>
BlockBase::source_blocks_t constant_block<T>::GetSourceBlocks() const
{
	return source_blocks_t();
}

template<typename T>
bool constant_block<T>::CanEvaluate() const
{
	return false;
}

template<typename T>
void constant_block<T>::Evaluate()
{
}
template<typename T>
node<T> constant_block<T>::add_output(T const &constant)
{
	outputs.emplace_back(this, constant);

	auto &back = outputs.back();
	back.value = constant;
	return back.GetNode();
}

// properties for code generation

template<typename T>
void FillValuesProperty(dfx::generator::Properties &, std::list<backend::OutputPin<T>> const &)
{
}

template<>
void FillValuesProperty<bool>(dfx::generator::Properties &properties, std::list<backend::OutputPin<bool>> const &outputs)
{
	int index = 0;
	for (auto &output : outputs)
		properties.SetInt("Constant", index++, output.value ? 1 : 0);
}

template<>
void FillValuesProperty<dynfix>(dfx::generator::Properties &properties, std::list<backend::OutputPin<dynfix>> const &outputs)
{
	int index = 0;
	for (auto &output : outputs) {

		auto value = output.value;

		for (int i = 0; i < dynfix::MAX_FIELDS; ++i) {
			properties.SetInt("Constant", index, i, value.data[i]);
		}

		++index;
	}
}


template<typename T>
void constant_block<T>::GetProperties(dfx::generator::Properties &properties) const
{
	properties.SetInt("BusWidth", (int)outputs.size());
	FillValuesProperty<T>(properties, outputs);
}


// explicit template implementations
template class constant_block<bool>;
template class constant_block<std::int32_t>;
template class constant_block<std::int64_t>;
template class constant_block<double>;
template class constant_block<dynfix>;


}
}
}
