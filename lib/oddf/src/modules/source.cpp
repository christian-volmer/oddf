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

	The 'source' module provides data during simulation on every clock
	cycle as determined by its 'ReadEnable' input. The available data can
	be specified through an std::vector before or during simulation.

*/

#include "../global.h"
#include "source.h"

namespace dfx {
namespace backend {
namespace blocks {

// replacemant<T> class required because std::vector does not work with bool.
template<typename S> struct replacement {
	typedef S type;
	static type cast(S const &value) { return value; }
};

template<> struct replacement<bool> {
	typedef char type;
	static bool cast(char const &value) { return value != 0; }
};

template<typename T>
class source_block : public BlockBase, private IStep {

private:
	
	InputPin<bool> readEnableInput;

	std::list<OutputPin<T>> outputs;
	OutputPin<bool> outputReadyOutput;

	std::vector<typename replacement<T>::type> values;

	std::vector<T> data;
	size_t dataPointer;
	bool DataReady;
	bool dataPeriodic;

	void read_next()
	{
		DataReady = false;

		size_t length = data.size();

		if (length == 0)
			return;

		size_t width = values.size();

		if (dataPeriodic) {

			for (auto &val : values) {

				if (dataPointer >= length)
					dataPointer = 0;

				val = data[dataPointer++];
			}
		}
		else {

			if (dataPointer + width > length)
				return;

			for (auto &val : values)
				val = data[dataPointer++];
		}

		DataReady = true;
	}

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		size_t width = values.size();
		auto outputIt = outputs.begin();

		if (DataReady) {

			// If we have enough data available, set OutputReady to true and copy the values to the output bus.
			outputReadyOutput.value = true;
			for (unsigned i = 0; i < width; ++i)
				(outputIt++)->value = replacement<T>::cast(values[i]);
		}
		else {

			// If there is no or insufficient data available, set OutputReady to false and set to output bus to 0.
			outputReadyOutput.value = false;
			for (unsigned i = 0; i < width; ++i)
				(outputIt++)->value = T();
		}
	}

	void Step() override
	{
		if (readEnableInput.GetValue()) {

			read_next();
			SetDirty();
		}
	}

	void AsyncReset() override
	{
		dataPointer = 0;
		read_next();
		SetDirty();
	}

	IStep *GetStep()
	{
		return this;
	}

	source_blocks_t GetSourceBlocks() const override
	{
		return source_blocks_t();
	}

	void set_data(std::vector<T> &&newData, bool periodic)
	{
		data = std::move(newData);
		dataPointer = 0;
		dataPeriodic = periodic;
		read_next();

		SetDirty();
	}

	friend class modules::Source<T>;

public:

	source_block(unsigned width, node<bool> const &readEnable) :
		BlockBase("source"),
		readEnableInput(this, readEnable),
		outputs(),
		outputReadyOutput(this, false),
		values(width, T()),
		data(),
		dataPointer(0),
		DataReady(false),
		dataPeriodic(false)
	{
		while (width-- > 0)
			outputs.emplace_back(this, T());

		read_next();
	}

	bus<T> get_output_bus()
	{
		bus<T> outputBus;

		for (auto &output : outputs)
			outputBus.append(output.GetNode());

		return outputBus;
	}

	node<bool> get_output_ready()
	{
		return outputReadyOutput.GetNode();
	}
};

}
}

namespace modules {

template<typename T> Source<T>::Source(int busWidth /* = 1 */)
{
	if (busWidth < 1)
		throw design_error("Source module: Parameter 'busWidth' must be greate than or equal to one.");

	auto &block = Design::GetCurrent().NewBlock<backend::blocks::source_block<T>>(busWidth, Inputs.ReadEnable);
	Block = &block;

	Outputs.DataBus = block.get_output_bus();
	Outputs.Data = Outputs.DataBus.first();
	Outputs.OutputReady = block.get_output_ready();
}

template<typename T> void Source<T>::SetData(class std::vector<T> const &data, bool periodic /* = false */)
{
	Block->set_data(std::vector<T>(data), periodic);
}

template<typename T> void Source<T>::SetData(class std::vector<T> &&data, bool periodic /* = false */)
{
	Block->set_data(std::move(data), periodic);
}

// explicit template implementations
template class Source<bool>;
template class Source<std::int32_t>;
template class Source<std::int64_t>;
template class Source<double>;

}
}
