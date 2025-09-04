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

#include <oddf/utility/BooleanSupport.h>
#include <oddf/utility/IntegerSupport.h>
#include <oddf/Exception.h>

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

template<typename T>
void *constant_block<T>::GetInterface(oddf::Uid const &iid)
{
	if (iid == oddf::Iid<oddf::design::blocks::backend::IConstantBlock>::value)
		return dynamic_cast<oddf::design::blocks::backend::IConstantBlock *>(this);
	else
		return backend::BlockBase::GetInterface(iid);
}

//
// IConstantBlock for bool
//

template<>
size_t constant_block<bool>::GetSize() const noexcept
{
	return 1;
}

template<>
void constant_block<bool>::Read(void *buffer, size_t bufferSize) const
{
	std::uint8_t value = outputs.front().value ? 1 : 0;
	oddf::utility::BooleanCopy(buffer, bufferSize, &value, sizeof(value));
}

//
// IConstantBlock for dynfix
//

template<>
size_t constant_block<dynfix>::GetSize() const noexcept
{
	auto wordWidth = outputs.front().GetNodeType().GetWordWidth();
	return (wordWidth + 7) / 8;
}

template<>
void constant_block<dynfix>::Read(void *buffer, size_t bufferSize) const
{
	auto const &output = outputs.front();
	auto nodeType = output.GetNodeType();

	oddf::utility::IntegerCopy(buffer, bufferSize, &outputs.front().value.data, sizeof(dynfix::data),
		nodeType.GetWordWidth(), nodeType.IsSigned());
}

//
// IConstantBlock for other types have not been implemented yet.
//

template<typename T>
size_t constant_block<T>::GetSize() const noexcept
{
	return 0;
}

template<typename T>
void constant_block<T>::Read(void * /* buffer */, size_t /* count */) const
{
	throw oddf::Exception(oddf::ExceptionCode::NotImplemented);
}

//
// Properties for code generation
//

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

} // namespace blocks
} // namespace backend
} // namespace dfx
