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

	Label blocks are used internally by the design framework to assign
	names to parts of the design.

*/

#include "../global.h"
#include "../generator/generator.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class label_block : public BlockBase {

private:

	bool isBus;
	std::string labelClass;
	std::string label;
	std::list<InputPin<T>> inputs;

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

	std::string GetInputPinName(int index) const override
	{
		if (isBus) {

			if (index >= 0 && index < (int)GetInputPins().size())
				return label;
		}
		else
			if (index == 0)
				return label;

		assert(false);
		return "<ERROR>";
	}

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		properties.SetInt("isBus", isBus);
		properties.SetString("class", labelClass);
	}


public:

	label_block(bool _isBus, std::string const &_labelClass, std::string const &_label) :
		BlockBase("$label"),
		isBus(_isBus),
		labelClass(_labelClass),
		label(_label),
		inputs()
	{
	}

	void add_node(node<T> const &input)
	{
		assert(isBus || (inputs.size() == 0));
		inputs.emplace_back(this, input);
	}
};

}
}

namespace blocks {

#define IMPLEMENT_LABEL_FUNCTION(_type_) \
	void _Label(node<_type_> const &input, std::string const &labelClass, std::string const &label) \
	{ \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::label_block<_type_>>(false, labelClass, label); \
		block.add_node(input); \
	} \
 \
	void _Label(bus_access<_type_> const &inputBus, std::string const &labelClass, std::string const &label) \
	{ \
		unsigned width = inputBus.width(); \
 \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::label_block<_type_>>(true, labelClass, label); \
 \
		for (unsigned i = 1; i <= width; ++i) \
			block.add_node(inputBus(i)); \
 	}

IMPLEMENT_LABEL_FUNCTION(bool)
IMPLEMENT_LABEL_FUNCTION(double)
IMPLEMENT_LABEL_FUNCTION(std::int32_t)
IMPLEMENT_LABEL_FUNCTION(std::int64_t)
IMPLEMENT_LABEL_FUNCTION(dynfix)

}
}
