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

	Random() returns a new random integer upon each clock cycle.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

static std::mt19937_64 randomEngine;

class random_block : public BlockBase, private IStep {

private:

	bool initialised;

	InputPin<bool> readEnableInput;
	InputPin<int> maxInput;

	std::list<OutputPin<int>> outputs;

	std::vector<int> values;

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t({ maxInput.GetDrivingBlock() });
	}

	void generate_next()
	{
		int max = maxInput.GetValue();
		std::uniform_int_distribution<> uniform(0, max);

		for (auto &val : values)
			val = uniform(randomEngine);

		initialised = true;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		if (!initialised)
			generate_next();

		size_t width = values.size();
		auto outputIt = outputs.begin();

		for (size_t i = 0; i < width; ++i)
			(outputIt++)->value = values[i];
	}

	void Step() override
	{
		if (readEnableInput.GetValue()) {

			generate_next();
			SetDirty();
		}
	}

	void AsyncReset()
	{
	}

	IStep *GetStep()
	{
		return this;
	}

	std::string GetInputPinName(int index) const override
	{
		if (index == 0)
			return "ReadEnable";
		else if (index == 1)
			return "Max";

		assert(false);
		return "<ERROR>";
	}

public:

	random_block(unsigned width, node<int> const &max, node<bool> const &readEnable) :
		BlockBase("random"),
		initialised(false),
		readEnableInput(this, readEnable),
		maxInput(this, max),
		outputs(),
		values(width, int())
	{
		while (width-- > 0)
			outputs.emplace_back(this, 0);
	}

	random_block(random_block const &) = delete;
	random_block &operator =(random_block const &) = delete;

	bus<int> get_output_bus()
	{
		bus<int> outputBus;

		for (auto &output : outputs)
			outputBus.append(output.GetNode());

		return outputBus;
	}
};

}
}

namespace blocks {

node<int> Random(node<int> max, node<bool> readEnable)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::random_block>(1, max, readEnable);
	return block.get_output_bus().first();
}

bus<int> Random(node<int> max, node<bool> readEnable, int width)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::random_block>(width, max, readEnable);
	return block.get_output_bus();
}

}
}
