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

#pragma once

#include "../bus.h"
#include "../inout.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T> class source_block;

}
}

namespace modules {

template<typename T>
class Source {

private:

	backend::blocks::source_block<T> *Block;

public:

	struct {

		inout::input_node<bool> ReadEnable;

	} Inputs;

	struct {

		inout::output_node<bool> OutputReady;
		inout::output_node<T> Data;
		inout::output_bus<T> DataBus;

	} Outputs;

	Source(int busWidth = 1);

	void SetData(class std::vector<T> const &data, bool periodic = false);
	void SetData(class std::vector<T> &&data, bool periodic = false);
};

}
}
