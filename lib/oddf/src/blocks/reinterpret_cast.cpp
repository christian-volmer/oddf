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

	ReinterpretCast() converts one number type to another based on the
	number's binary representation.

*/

#include "../global.h"

namespace dfx {
namespace backend {
namespace blocks {

static void ReinterpretCastImpl(bool &dest, std::int32_t const &source)
{
	dest = (source & 1) != 0;
}

static void ReinterpretCastImpl(bool &dest, std::int64_t const &source)
{
	dest = (source & 1) != 0;
}

static void ReinterpretCastImpl(bool &dest, dynfix const &source)
{
	dest = (source.data[0] & 1) != 0;
}


static void ReinterpretCastImpl(std::int32_t &dest, bool const &source)
{
	dest = source;
}

static void ReinterpretCastImpl(std::int32_t &dest, std::int64_t const &source)
{
	dest = static_cast<std::int32_t>(source & 0xffffffff);
}

static void ReinterpretCastImpl(std::int32_t &dest, dynfix const &source)
{
	dynfix temp(false, source.GetWordWidth(), 0);
	source.Copy(temp);
	temp.OverflowWrapAround();
	dest = temp.data[0];
}


static void ReinterpretCastImpl(std::int64_t &dest, bool const &source)
{
	dest = source;
}

static void ReinterpretCastImpl(std::int64_t &dest, std::int32_t const &source)
{
	dest = source;
}

static void ReinterpretCastImpl(std::int64_t &dest, double const &source)
{
	dest = reinterpret_cast<std::int64_t const &>(source);
}

static void ReinterpretCastImpl(std::int64_t &dest, dynfix const &source)
{
	dynfix temp(false, source.GetWordWidth(), 0);
	source.Copy(temp);
	temp.OverflowWrapAround();
	dest = (static_cast<std::int64_t>(temp.data[0]) & 0xffffffff) | (static_cast<std::int64_t>(temp.data[1]) << 32);
}


static void ReinterpretCastImpl(double &dest, std::int64_t const &source)
{
	dest = reinterpret_cast<double const &>(source);
}

static void ReinterpretCastImpl(double &dest, dynfix const &source)
{
	std::int64_t temp = (static_cast<std::int64_t>(source.data[0]) & 0xffffffff) | (static_cast<std::int64_t>(source.data[1]) << 32);
	dest = reinterpret_cast<double const &>(temp);
}


static void ReinterpretCastImpl(dynfix &dest, bool const &source)
{
	dest.data[0] = source;
	for (int i = 1; i < dynfix::MAX_FIELDS; ++i)
		dest.data[i] = 0;
	dest.OverflowWrapAround();
}

static void ReinterpretCastImpl(dynfix &dest, std::int32_t const &source)
{
	dest.data[0] = source;
	for (int i = 1; i < dynfix::MAX_FIELDS; ++i)
		dest.data[i] = 0;
	dest.OverflowWrapAround();
}

static void ReinterpretCastImpl(dynfix &dest, std::int64_t const &source)
{
	dest.data[0] = (std::int32_t)(source & 0xffffffff);
	dest.data[1] = (std::int32_t)((source >> 32) & 0xffffffff);
	for (int i = 2; i < dynfix::MAX_FIELDS; ++i)
		dest.data[i] = 0;
	dest.OverflowWrapAround();
}

static void ReinterpretCastImpl(dynfix &dest, double const &source)
{
	std::int64_t temp = reinterpret_cast<std::int64_t const &>(source);

	dest.data[0] = (std::int32_t)(temp & 0xffffffff);
	dest.data[1] = (std::int32_t)((temp >> 32) & 0xffffffff);
	for (int i = 2; i < dynfix::MAX_FIELDS; ++i)
		dest.data[i] = 0;
	dest.OverflowWrapAround();
}

static void ReinterpretCastImpl(dynfix &dest, dynfix const &source)
{
	dynfix temp(false, source.GetWordWidth(), 0);
	source.Copy(temp);
	temp.OverflowWrapAround();
	temp.Copy(dest);
	dest.OverflowWrapAround();
}

template<typename destT, typename sourceT> class reinterpret_cast_block : public BlockBase {

private:

	destT outputTemplate;

	struct path {

		InputPin<sourceT> input;
		OutputPin<destT> output;

		path(reinterpret_cast_block *block, node<sourceT> const &inputNode) :
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
		for (auto &path : paths)
			ReinterpretCastImpl(path.output.value, path.input.GetValue());
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

	reinterpret_cast_block(destT const &outputTemplate) :
		BlockBase("reinterpret_cast"),
		outputTemplate(outputTemplate),
		paths()
	{
	}

	node<destT> add_node(node<sourceT> const &operand)
	{
		paths.emplace_back(this, operand);
		return paths.back().output.GetNode();
	}

	bus<destT> add_bus(bus_access<sourceT> const &operand)
	{
		unsigned width = operand.width();

		bus<destT> outputBus;
		for (unsigned i = 1; i <= width; ++i)
			outputBus.append(add_node(operand(i)));

		return outputBus;
	}
};

}
}

namespace blocks {

template<> bus<bool> ReinterpretCast(bool const &, bus_access<bool> const &input)
{
	return input;
}

template<> bus<bool> ReinterpretCast(bool const &outputTemplate, bus_access<std::int32_t> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<bool, std::int32_t>>(outputTemplate).add_bus(input);
}

template<> bus<bool> ReinterpretCast(bool const &outputTemplate, bus_access<std::int64_t> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<bool, std::int64_t>>(outputTemplate).add_bus(input);
}

// template<> bus<bool> ReinterpretCast(bool const &, bus_access<double> const &);
template<> bus<bool> ReinterpretCast(bool const &outputTemplate, bus_access<dynfix> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<bool, dynfix>>(outputTemplate).add_bus(input);
}


template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &outputTemplate, bus_access<bool> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<std::int32_t, bool>>(outputTemplate).add_bus(input);
}

template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &, bus_access<std::int32_t> const &input)
{
	return input;
}

template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &outputTemplate, bus_access<std::int64_t> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<std::int32_t, std::int64_t>>(outputTemplate).add_bus(input);
}

//template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &, bus_access<double> const &);
template<> bus<std::int32_t> ReinterpretCast(std::int32_t const &outputTemplate, bus_access<dynfix> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<std::int32_t, dynfix>>(outputTemplate).add_bus(input);
}


template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &outputTemplate, bus_access<bool> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<std::int64_t, bool>>(outputTemplate).add_bus(input);
}

template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &outputTemplate, bus_access<std::int32_t> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<std::int64_t, std::int32_t>>(outputTemplate).add_bus(input);
}

template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &, bus_access<std::int64_t> const &input)
{
	return input;
}

template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &outputTemplate, bus_access<double> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<std::int64_t, double>>(outputTemplate).add_bus(input);
}

template<> bus<std::int64_t> ReinterpretCast(std::int64_t const &outputTemplate, bus_access<dynfix> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<std::int64_t, dynfix>>(outputTemplate).add_bus(input);
}


//template<> bus<double> ReinterpretCast(double const &, bus_access<bool> const &);
//template<> bus<double> ReinterpretCast(double const &, bus_access<std::int32_t> const &);
template<> bus<double> ReinterpretCast(double const &outputTemplate, bus_access<std::int64_t> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<double, std::int64_t>>(outputTemplate).add_bus(input);
}

template<> bus<double> ReinterpretCast(double const &, bus_access<double> const &input)
{
	return input;
}

template<> bus<double> ReinterpretCast(double const &outputTemplate, bus_access<dynfix> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<double, dynfix>>(outputTemplate).add_bus(input);
}


template<> bus<dynfix> ReinterpretCast(dynfix const &outputTemplate, bus_access<bool> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<dynfix, bool>>(outputTemplate).add_bus(input);
}

template<> bus<dynfix> ReinterpretCast(dynfix const &outputTemplate, bus_access<std::int32_t> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<dynfix, std::int32_t>>(outputTemplate).add_bus(input);
}

template<> bus<dynfix> ReinterpretCast(dynfix const &outputTemplate, bus_access<std::int64_t> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<dynfix, std::int64_t>>(outputTemplate).add_bus(input);
}

template<> bus<dynfix> ReinterpretCast(dynfix const &outputTemplate, bus_access<double> const &input)
{
	return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<dynfix, double>>(outputTemplate).add_bus(input);
}

template<> bus<dynfix> ReinterpretCast(dynfix const &outputTemplate, bus_access<dynfix> const &input)
{
	types::TypeDescription targetTypeDesc = types::GetDescription(outputTemplate);
	int width = input.width();

	// Check if one of the input has a type different from the target type. Then we instantiate the converter block. This is the typical scenario.
	for (int i = 0; i < width; ++i)
		if (types::GetDescription(input[i].GetDriver()->value) != targetTypeDesc)
			return Design::GetCurrent().NewBlock<backend::blocks::reinterpret_cast_block<dynfix, dynfix>>(outputTemplate).add_bus(input);

	// In case all inputs already have the target type (not the intended use case, but could happen), we just pass them through.
	return input;
}


}
}
