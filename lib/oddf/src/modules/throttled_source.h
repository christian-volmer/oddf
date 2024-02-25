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

	Like the normal 'source' module but instead of having a read-enable
	input, the data is provided at an average speed specified during data
	initialisation.

*/

#pragma once

#include "../bus.h"
#include "../inout.h"
#include "source.h"

namespace dfx {
namespace modules {

template<typename T>
class ThrottledSource {

private:

	dfx::modules::Source<int> mResetGenerator;
	dfx::modules::Source<T> mSource;
	double mIncrement;

public:

	struct {

		inout::output_node<bool> Valid;
		inout::output_node<T> Data;
		inout::output_bus<T> DataBus;

	} Outputs;

	ThrottledSource(int busWidth = 1);

	void SetData(class std::vector<T> const &data, double increment, bool periodic = false);
	void SetData(class std::vector<T> &&data, double increment, bool periodic = false);
};

}
}
