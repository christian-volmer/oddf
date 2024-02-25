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

#pragma once

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class sink_block;

}
}

namespace modules {

template<typename T>
class Sink {

private:
	backend::blocks::sink_block<T> *Block;

public:

	struct {

		inout::input_node<bool> WriteEnable;
		inout::input_node<T> Data;

	} Inputs;

	class std::vector<T> const &GetData() const;
	void Clear();

	Sink();
};



template<typename T>
class BusSink {

private:
	backend::blocks::sink_block<T> *Block;

public:

	struct {

		inout::input_node<bool> WriteEnable;
		inout::input_bus<T> Data;

	} Inputs;

	class std::vector<T> const &GetData() const;
	void Clear();

	BusSink(int busWidth);
};


}
}
