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

namespace oddf::simulator::common::backend {

struct SimulatorInstruction;

template<typename instructionT = SimulatorInstruction>
using SimulatorInstructionFunction = void(instructionT *);

struct SimulatorInstruction {

	SimulatorInstructionFunction<SimulatorInstruction> *m_function;

	union {

		size_t m_size;
		SimulatorInstruction *m_next;
	};

	SimulatorInstruction() :
		m_function(), m_size()
	{
	}

	SimulatorInstruction(SimulatorInstruction const &) = delete;
	void operator=(SimulatorInstruction const &) = delete;

	void Execute()
	{
		m_function(this);
	}

	template<typename T>
	T &GetRecord(size_t offset)
	{
		return *reinterpret_cast<T *>(reinterpret_cast<char *>(this) + offset);
	}

	template<typename T>
	T const &GetRecord(size_t offset) const
	{
		return *reinterpret_cast<T const *>(reinterpret_cast<char const *>(this) + offset);
	}
};

} // namespace oddf::simulator::common::backend
