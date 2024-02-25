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

#include "../global.h"
#include "throttled_source.h"

namespace dfx {
namespace modules {

template<typename T> ThrottledSource<T>::ThrottledSource(int busWidth /* = 1 */) :
	mResetGenerator(),
	mSource(busWidth),
	mIncrement(1.0)
{
	mResetGenerator.Inputs.ReadEnable <<= blocks::Constant(true);

	node<bool> init = mResetGenerator.Outputs.OutputReady;
	node<double> increment = blocks::Signal(&mIncrement);

	forward_node<double> accumulator;
	node<double> accumulator_next = blocks::Decide(init, 1.0, accumulator + increment);
	node<bool> overflow = accumulator_next >= 1.0;
	accumulator_next = blocks::Decide(overflow, accumulator_next - 1.0, accumulator_next);
	accumulator <<= DFX_DELAY(accumulator_next);

	Outputs.Valid = mSource.Outputs.OutputReady && overflow;
	Outputs.Data = mSource.Outputs.Data;
	Outputs.DataBus = mSource.Outputs.DataBus;
	mSource.Inputs.ReadEnable <<= overflow;
}

template<typename T> void ThrottledSource<T>::SetData(class std::vector<T> const &data, double increment, bool periodic /* = false */)
{
	mIncrement = increment;
	mSource.SetData(data, periodic);
	mResetGenerator.SetData(std::vector<int>{ 0 }, false);
}

template<typename T> void ThrottledSource<T>::SetData(class std::vector<T> &&data, double increment, bool periodic /* = false */)
{
	mIncrement = increment;
	mSource.SetData(std::move(data), periodic);
	mResetGenerator.SetData(std::vector<int>{ 0 }, false);
}

// explicit template implementations
template class ThrottledSource<bool>;
template class ThrottledSource<std::int32_t>;
template class ThrottledSource<std::int64_t>;
template class ThrottledSource<double>;

}
}
