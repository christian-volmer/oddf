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

	Decide() provides nested if-then-else functionality as a design
	block. Implementation for 'dynfix'.

*/

#include "../global.h"
#include "../generator/properties.h"

namespace dfx {
namespace backend {
namespace blocks {

class decide_block_dynfix : public BlockBase {

private:

	InputPin<bool> decisionInput;

	struct Path {

		InputPin<dynfix> trueInput;
		InputPin<dynfix> falseInput;
		OutputPin<dynfix> output;
		int trueShift;
		int falseShift;

		Path(BlockBase *block, node<dynfix> const &trueNode, int trueShift, node<dynfix> const &falseNode, int falseShift, dynfix const &outputTemplate) :
			trueInput(block, trueNode),
			falseInput(block, falseNode),
			output(block, outputTemplate),
			trueShift(trueShift),
			falseShift(falseShift)
		{
			assert(trueShift >= 0);
			assert(falseShift >= 0);
			assert((trueShift == 0) || (falseShift == 0));
		}
	};

	std::list<Path> paths;
	bool allTheSameType;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;

		blocks.insert(decisionInput.GetDrivingBlock());

		for (auto &path : paths) {

			blocks.insert(path.trueInput.GetDrivingBlock());
			blocks.insert(path.falseInput.GetDrivingBlock());
		}

		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	std::string GetInputPinName(int index) const override
	{
		if (index == 0)
			return "Decision";
		else if (index >= 1 && index < (int)GetInputPins().size())
			return ((index - 1) % 2 == 0 ? "True" : "False") + std::to_string((index - 1) / 2);

		assert(false);
		return "<ERROR>";
	}

	void Evaluate() override
	{
		if (decisionInput.GetValue()) {

			for (auto &p : paths)
				p.trueInput.GetValue().CopyShiftLeft(p.output.value, p.trueShift);
		}
		else {

			for (auto &p : paths)
				p.falseInput.GetValue().CopyShiftLeft(p.output.value, p.falseShift);
		}
	}

	std::string GetOutputPinDescription(int index, int &groupIndex, int &busSize, int &busIndex) const override
	{
		int length = (int)paths.size();

		if (allTheSameType && length > 1 && index >= 0 && index < length) {

			groupIndex = 0;
			busSize = length;
			busIndex = index;
			return "Out";
		}

		return BlockBase::GetOutputPinDescription(index, groupIndex, busSize, busIndex);
	}

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		int index = 0;
		for (auto const &path : paths) {

			properties.SetInt("TrueShift", index, path.trueShift);
			properties.SetInt("FalseShift", index, path.falseShift);
			++index;
		}
	}

public:

	decide_block_dynfix(node<bool> decisionNode) :
		BlockBase("decide"),
		decisionInput(this, decisionNode),
		paths(),
		allTheSameType(true)
	{
	}

	node<dynfix> add_path(node<dynfix> const &trueInput, node<dynfix> const &falseInput)
	{
		dynfix trueTemplate = types::DefaultFrom(trueInput.GetDriver()->value);
		dynfix falseTemplate = types::DefaultFrom(falseInput.GetDriver()->value);

		bool trueSigned = trueTemplate.IsSigned();
		int trueWordWidth = trueTemplate.GetWordWidth();
		int trueFraction = trueTemplate.GetFraction();
		int trueShift = 0;

		bool falseSigned = falseTemplate.IsSigned();
		int falseWordWidth = falseTemplate.GetWordWidth();
		int falseFraction = falseTemplate.GetFraction();
		int falseShift = 0;

		// Signedness
		if (trueSigned && !falseSigned) {

			falseSigned = true;
			++falseWordWidth;
		}
		else if (falseSigned && !trueSigned) {

			trueSigned = true;
			++trueWordWidth;
		}

		// Fraction
		if (trueFraction > falseFraction) {

			falseWordWidth += trueFraction - falseFraction;
			falseShift += trueFraction - falseFraction;
			falseFraction = trueFraction;
		}
		else if (falseFraction > trueFraction) {

			trueWordWidth += falseFraction - trueFraction;
			trueShift += falseFraction - trueFraction;
			trueFraction = falseFraction;
		}

		// Word width
		if (trueWordWidth > falseWordWidth)
			falseWordWidth = trueWordWidth;
		else if (falseWordWidth > trueWordWidth)
			trueWordWidth = falseWordWidth;

		assert(trueSigned == falseSigned);
		assert(trueWordWidth == falseWordWidth);
		assert(trueFraction == falseFraction);

		dynfix outputTemplate(trueSigned, trueWordWidth, trueFraction);

		if (!paths.empty())
			allTheSameType = allTheSameType && (types::GetDescription(outputTemplate) == paths.back().output.GetType());

		paths.emplace_back(this, trueInput, trueShift, falseInput, falseShift, outputTemplate);
		return paths.back().output.GetNode();
	}

	bus<dynfix> add_bus(bus_access<dynfix> const &trueInput, bus_access<dynfix> const &falseInput)
	{
		int width = trueInput.width();

		if (falseInput.width() != width)
			throw design_error(GetFullName() + ": Operands of bus operations must have the same width.");

		bus<dynfix> outputBus;
		for (int i = 0; i < width; ++i)
			outputBus.append(add_path(trueInput[i], falseInput[i]));

		return outputBus;
	}
};

}
}

namespace blocks {

node<dynfix> Decide(node<bool> const &decisionNode, node<dynfix> const &trueInput, node<dynfix> const &falseInput)
{
	return Design::GetCurrent().NewBlock<backend::blocks::decide_block_dynfix>(decisionNode).add_path(trueInput, falseInput);
}

bus<dynfix> Decide(node<bool> const &decisionNode, bus_access<dynfix> const &trueInput, bus_access<dynfix> const &falseInput)
{
	return Design::GetCurrent().NewBlock<backend::blocks::decide_block_dynfix>(decisionNode).add_bus(trueInput, falseInput);
}

node<dynfix> Decide(node<bool> const &decisionNode, node<dynfix> const &trueInput, dynfix const &falseInput)
{
	return Decide(decisionNode, trueInput, Constant(falseInput));
}

node<dynfix> Decide(node<bool> const &decisionNode, dynfix const &trueInput, node<dynfix> const &falseInput)
{
	return Decide(decisionNode, Constant(trueInput), falseInput);
}

node<dynfix> Decide(node<bool> const &decisionNode, dynfix const &trueInput, dynfix const &falseInput)
{
	return Decide(decisionNode, Constant(trueInput), Constant(falseInput));
}

bus<dynfix> Decide(node<bool> const &decisionNode, bus_access<dynfix> const &trueInput, node<dynfix> const &falseInput)
{
	return Decide(decisionNode, trueInput, bus<dynfix>(falseInput, trueInput.width()));
}

bus<dynfix> Decide(node<bool> const &decisionNode, node<dynfix> const &trueInput, bus_access<dynfix> const &falseInput)
{
	return Decide(decisionNode, bus<dynfix>(trueInput, falseInput.width()), falseInput);
}

bus<dynfix> Decide(node<bool> const &decisionNode, bus_access<dynfix> const &trueInput, dynfix const &falseInput)
{
	return Decide(decisionNode, trueInput, bus<dynfix>(Constant(falseInput), trueInput.width()));
}

bus<dynfix> Decide(node<bool> const &decisionNode, dynfix const &trueInput, bus_access<dynfix> const &falseInput)
{
	return Decide(decisionNode, bus<dynfix>(Constant(trueInput), falseInput.width()), falseInput);
}

}
}
