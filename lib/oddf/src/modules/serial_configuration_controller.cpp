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

	Implementation of the configuration::IController interfaces. Provides
	access a serial interface similar to AMBA/APB.

*/

#include "../global.h"
#include "serial_configuration.h"

#include <memory>
#include <cstring>

namespace dfx {
namespace modules {

SerialConfigurationController::SerialConfigurationController(int theTimeOut) :
	readAcknowledge(),
	readData(),
	timeOut(theTimeOut),
	clearAll(), 
	address(), 
	writeEnable(), 
	writeData(), 
	readRequest(),
	simulator(nullptr)
{
	namespace b = blocks;

	readAcknowledge = b::Probe(Inputs.ReadAcknowledge);
	readData = b::Probe(b::FloorCast(*readData, Inputs.ReadData));

	Outputs.ClearAll = b::Signal(&clearAll);
	Outputs.Address = b::FloorCast<ufix<configuration::AddressWidth>>(b::Signal(&address));
	Outputs.WriteEnable = b::Signal(&writeEnable);
	Outputs.WriteData = b::FloorCast<configuration::DataT>(b::Signal(&writeData));
	Outputs.ReadRequest = b::Signal(&readRequest);
}

void SerialConfigurationController::AttachSimulator(dfx::Simulator &theSimulator)
{
	simulator = &theSimulator;
}

void SerialConfigurationController::Write(int theAddress, std::uint32_t const *values, int count)
{
	if (!simulator)
		throw std::runtime_error("SerialConfigurationController::Write(): must attach a simulator using AttachSimulator() before calling this function.");

	writeEnable = true;
	for (int i = 0; i < count; ++i) {

		address = theAddress + i;

		std::memcpy(&writeData, &values[i], sizeof(std::int32_t));

		simulator->Run(1);
	}

	writeEnable = false;
}

void SerialConfigurationController::Read(int theAddress, std::uint32_t *values, int count)
{
	if (!simulator)
		throw std::runtime_error("SerialConfigurationController::Read(): must attach a simulator using AttachSimulator() before calling this function.");

	int reqIndex, ackIndex = 0;

	readRequest = true;
	for (reqIndex = 0; reqIndex < count; ++reqIndex) {

		address = theAddress + reqIndex;
		simulator->Run(1);

		if (*readAcknowledge) {

			std::memcpy(&values[ackIndex], readData, sizeof(std::uint32_t));
			++ackIndex;
		}
	}

	readRequest = false;

	int timeOutCounter = 0;
	while (ackIndex < count) {

		simulator->Run(1);

		if (*readAcknowledge) {

			std::memcpy(&values[ackIndex], readData, sizeof(std::uint32_t));
			++ackIndex;
		}
		else
			if (++timeOutCounter >= timeOut)
				throw std::runtime_error("SerialConfigurationController::Read(): read response timed out.");
	}
}

void SerialConfigurationController::ClearAll()
{
	if (!simulator)
		throw std::runtime_error("SerialConfigurationController::Write(): must attach a simulator using AttachSimulator() before calling this function.");

	clearAll = true;
	simulator->Run(1);
	clearAll = false;
}

}
}
