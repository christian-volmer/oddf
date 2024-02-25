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

	FloorCast() uses rounding towards negative infinity to convert from
	one number type to another.

*/

#include "../global.h"

namespace dfx {
namespace blocks {

//
// Conversion to bool
//

template<> bus<bool> FloorCast(bool const &, bus_access<bool> const &input, CastMode)
{
	return input;
}

// template<> bus<bool> FloorCast(bool const &, bus_access<std::int32_t> const &);
// template<> bus<bool> FloorCast(bool const &, bus_access<std::int64_t> const &);
// template<> bus<bool> FloorCast(bool const &, bus_access<double> const &);
// template<> bus<bool> FloorCast(bool const &, bus_access<dynfix> const &);


//
// Conversion to int32
//

template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<bool> const &input, CastMode)
{
	return Function(input, [](bool value) { return value ? std::int32_t(1) : std::int32_t(0); });
}

template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<std::int32_t> const &input, CastMode)
{
	return input;
}

template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<std::int64_t> const &input, CastMode castMode)
{
	if (castMode == CastMode::WrapAround) {

		return Function(input, [](std::int64_t value) {

			return static_cast<std::int32_t>(value);
		});
	}
	else { /* castMode == CastMode::Saturate */

		return Function(input, [](std::int64_t value) {

			if (value >= INT32_MAX)
				return INT32_MAX;
			else if (value <= INT32_MIN)
				return INT32_MIN;
			else
				return static_cast<std::int32_t>(value);
		});
	}
}

template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<double> const &input, CastMode castMode)
{
	if (castMode == CastMode::WrapAround) {

		return Function(input, [](double value) {

			return static_cast<std::int32_t>(std::floor(value));
		});
	}
	else { /* castMode == CastMode::Saturate */

		return Function(input, [](double value) {

			double floorValue = std::floor(value);

			if (floorValue >= INT32_MAX)
				return INT32_MAX;
			else if (floorValue <= INT32_MIN)
				return INT32_MIN;
			else
				return static_cast<std::int32_t>(floorValue);
		});
	}
}

template<> bus<std::int32_t> FloorCast(std::int32_t const &, bus_access<dynfix> const &input, CastMode castMode)
{
	return ReinterpretCast<std::int32_t>(FloorCast<sfix<32>>(input, castMode));
}


//
// Conversion to int64
//

template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<bool> const &input, CastMode)
{
	return dfx::blocks::Function(input, [](bool value) { return value ? std::int64_t(1) : std::int64_t(0); });
}

template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<std::int32_t> const &input, CastMode)
{
	return dfx::blocks::Function(input, [](std::int32_t value) { return static_cast<std::int64_t>(value); });
}

template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<std::int64_t> const &input, CastMode)
{
	return input;
}

template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<double> const &input, CastMode castMode)
{
	if (castMode == CastMode::WrapAround) {

		return Function(input, [](double value) {

			return static_cast<std::int64_t>(std::floor(value));
		});
	}
	else { /* castMode == CastMode::Saturate */

		return Function(input, [](double value) {

			double floorValue = std::floor(value);

			if (floorValue >= INT64_MAX)
				return INT64_MAX;
			else if (floorValue <= INT64_MIN)
				return INT64_MIN;
			else
				return static_cast<std::int64_t>(floorValue);
		});
	}
}

template<> bus<std::int64_t> FloorCast(std::int64_t const &, bus_access<dynfix> const &input, CastMode castMode)
{
	return ReinterpretCast<std::int64_t>(FloorCast<sfix<64>>(input, castMode));
}


//
// Conversion to double
//

template<> bus<double> FloorCast(double const &, bus_access<bool> const &input, CastMode)
{
	return dfx::blocks::Function(input, [](bool value) { return static_cast<double>(value); });
}

template<> bus<double> FloorCast(double const &, bus_access<std::int32_t> const &input, CastMode)
{
	return dfx::blocks::Function(input, [](std::int32_t value) { return static_cast<double>(value); });
}

template<> bus<double> FloorCast(double const &, bus_access<std::int64_t> const &input, CastMode)
{
	return dfx::blocks::Function(input, [](std::int64_t value) { return static_cast<double>(value); });
}

template<> bus<double> FloorCast(double const &, bus_access<double> const &input, CastMode)
{
	return input;
}

template<> bus<double> FloorCast(double const &, bus_access<dynfix> const &input, CastMode)
{
	return dfx::blocks::Function(input, [](dynfix const &value) { return static_cast<double>(value); });
}

}

namespace backend {
namespace blocks {


//
// Conversion to dynfix
//

template<typename sourceT> class floor_cast_block_dynfix : public BlockBase {

private:

	dynfix outputTemplate;

	struct path {

		InputPin<sourceT> input;
		OutputPin<dynfix> output;

		path(floor_cast_block_dynfix *block, node<sourceT> const &inputNode) :
			input(block, inputNode),
			output(block, block->outputTemplate)
		{
		}
	};

	std::list<path> paths;

	source_blocks_t GetSourceBlocks() const override
	{
		source_blocks_t blocks;

		for (auto &path : paths)
			blocks.insert(path.input.GetDrivingBlock());

		return blocks;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		for (auto &p : paths) {

			// TODO: This is generic, but slower than necessary for non-dynfix source type.

			dynfix source(p.input.GetValue());
			int align = outputTemplate.GetFraction() - source.GetFraction();

			if (align >= 0)
				source.CopyShiftLeft(p.output.value, align);
			else
				source.CopyShiftRight(p.output.value, -align);

			p.output.value.OverflowWrapAround();
		}
	}

	std::string GetOutputPinDescription(int index, int &groupIndex, int &busSize, int &busIndex) const override
	{
		int length = (int)paths.size();

		if (length > 1 && index >= 0 && index < length) {

			groupIndex = 0;
			busSize = length;
			busIndex = index;
			return "Out";
		}

		return BlockBase::GetOutputPinDescription(index, groupIndex, busSize, busIndex);
	}


public:

	floor_cast_block_dynfix(dynfix const &outputTemplate) :
		BlockBase("floor_cast"),
		outputTemplate(outputTemplate),
		paths()
	{
	}

	node<dynfix> add_node(node<sourceT> const &operand)
	{
		paths.emplace_back(this, operand);
		return paths.back().output.GetNode();
	}

	bus<dynfix> add_bus(bus_access<sourceT> const &operand)
	{
		unsigned width = operand.width();

		bus<dynfix> outputBus;
		for (unsigned i = 1; i <= width; ++i)
			outputBus.append(add_node(operand(i)));

		return outputBus;
	}
};

}
}

namespace blocks {

//template<> bus<dynfix> FloorCast(dynfix const &, bus_access<bool> const &);

template<> bus<dynfix> FloorCast(dynfix const &outputTemplate, bus_access<std::int32_t> const &input, CastMode castMode)
{
	if (castMode != CastMode::WrapAround)
		throw design_error("dfx::blocks::FloorCast: cast from 'int32_t' to 'dynfix' supports 'CastMode::WrapAround' only.");

	auto &block = Design::GetCurrent().NewBlock<backend::blocks::floor_cast_block_dynfix<std::int32_t>>(outputTemplate);
	return block.add_bus(input);
}

template<> bus<dynfix> FloorCast(dynfix const &outputTemplate, bus_access<std::int64_t> const &input, CastMode castMode)
{
	if (castMode != CastMode::WrapAround)
		throw design_error("dfx::blocks::FloorCast: cast from 'int64_t' to 'dynfix' supports 'CastMode::WrapAround' only.");

	auto &block = Design::GetCurrent().NewBlock<backend::blocks::floor_cast_block_dynfix<std::int64_t>>(outputTemplate);
	return block.add_bus(input);
}

template<> bus<dynfix> FloorCast(dynfix const &outputTemplate, bus_access<double> const &input, CastMode castMode)
{
	if (castMode != CastMode::WrapAround)
		throw design_error("dfx::blocks::FloorCast: cast from 'double' to 'dynfix' supports 'CastMode::WrapAround' only.");

	auto &block = Design::GetCurrent().NewBlock<backend::blocks::floor_cast_block_dynfix<double>>(outputTemplate);
	return block.add_bus(input);
}

template<> bus<dynfix> FloorCast(dynfix const &outputTemplate, bus_access<dynfix> const &input, CastMode castMode)
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::floor_cast_block_dynfix<dynfix>>(outputTemplate);
	auto result = block.add_bus(input);

	if (castMode == CastMode::Saturate) {

		// TODO: check special case when casting to a larger type. Then we can skip the saturation logic.
		node<dynfix> min = Constant(outputTemplate.GetMin());
		node<dynfix> max = Constant(outputTemplate.GetMax());

		bus<bool> underflows = input < bus<dynfix>(min, input.width());
		bus<bool> overflows = input > bus<dynfix>(max, input.width());

		for (int i = 0; i < result.width(); ++i) {
			result[i] = Decide(
				overflows[i], max,
				underflows[i], min,
				result[i]);
		}
	}

	return result;
}

}
}
