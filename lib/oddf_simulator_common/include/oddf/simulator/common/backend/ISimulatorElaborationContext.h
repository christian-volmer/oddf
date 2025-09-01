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

    <no description>

*/

#pragma once

#include <memory>

namespace oddf::simulator::common::backend {

class SimulatorBlockBase;
class SimulatorBlockInput;
class SimulatorBlockOutput;

class ISimulatorElaborationContext {

public:

	virtual ~ISimulatorElaborationContext() = default;

	virtual void AddSimulatorBlock(std::unique_ptr<SimulatorBlockBase> &&block) = 0;
	virtual void RemoveThisBlock() = 0;

	// Disconnects `fromInput` and connects its former driver to `toInput`.
	virtual void TransferConnectivity(SimulatorBlockInput const &fromInput, SimulatorBlockInput const &toInput) = 0;

	// Reconnects all inputs driven by `fromOutput` to `toOutput`. Throws
	// unless the two outputs have identical types.
	virtual void TransferConnectivity(SimulatorBlockOutput const &fromOutput, SimulatorBlockOutput const &toOutput) = 0;

	// Disconnects the specified input. The input must belong to the block that
	// is currently under elaboration.
	virtual void DisconnectInput(SimulatorBlockInput const &input) = 0;

	template<typename T, typename... argsTs>
	T &AddSimulatorBlock(argsTs &&...args)
	{
		auto uptr = std::make_unique<T>(std::forward<argsTs>(args)...);
		T *p = uptr.get();
		AddSimulatorBlock(std::move(uptr));
		return *p;
	}
};

} // namespace oddf::simulator::common::backend
