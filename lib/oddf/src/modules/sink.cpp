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

	The 'sink' module consumes data during simulation on every clock
	cycle as determined by its 'writeEnableInput'. Collected data can be
	copied to an std::vector after simulation.

*/

#include "../global.h"
#include "sink.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T>
class sink_block : public BlockBase, private IStep {

private:

	InputPin<bool> writeEnableInput;

	std::list<InputPin<T>> inputs;

	std::vector<T> Data;

	void write_next()
	{
		for (auto &input : inputs)
			Data.push_back(input.GetValue());
	}

	bool CanEvaluate() const override
	{
		return false;
	}

	void Evaluate() override
	{
	}

	void Step() override
	{
		if (writeEnableInput.GetValue())
			write_next();
	}

	void AsyncReset() override
	{
	}

	IStep *GetStep()
	{
		return this;
	}

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t();
	}

	friend class modules::Sink<T>;
	friend class modules::BusSink<T>;

public:

	sink_block(node<bool> const &writeEnable) :
		BlockBase("sink"),
		writeEnableInput(this, writeEnable),
		inputs(),
		Data()
	{
	}

	void add_input(node<T> const &node)
	{
		inputs.emplace_back(this, node);
	}
};

}
}

namespace modules {

template<typename T> Sink<T>::Sink()
{
	auto &block = Design::GetCurrent().NewBlock<backend::blocks::sink_block<T>>(Inputs.WriteEnable);
	Block = &block;
	block.add_input(Inputs.Data);
}

template<typename T> BusSink<T>::BusSink(int busWidth)
{
	if (busWidth < 1)
		throw design_error("Sink module: Parameter 'busWidth' must be greate than or equal to one.");

	auto &block = Design::GetCurrent().NewBlock<backend::blocks::sink_block<T>>(Inputs.WriteEnable);
	Block = &block;

	Inputs.Data.create(busWidth);
	for (int i = 1; i <= busWidth; ++i)
		block.add_input(Inputs.Data(i));
}

template<typename T> std::vector<T> const &Sink<T>::GetData() const
{
	return Block->Data;
}

template<typename T> std::vector<T> const &BusSink<T>::GetData() const
{
	return Block->Data;
}

template<typename T> void Sink<T>::Clear()
{
	Block->Data.clear();
}

template<typename T> void BusSink<T>::Clear()
{
	Block->Data.clear();
}




// explicit template implementations
template class Sink<bool>;
template class Sink<std::int32_t>;
template class Sink<std::int64_t>;
template class Sink<double>;

template class BusSink<bool>;
template class BusSink<std::int32_t>;
template class BusSink<std::int64_t>;
template class BusSink<double>;


}
}
