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

	Implementations of the configuration::IBuilder and
	configuration::IController interfaces. Provides access through a
	serial interface similar to AMBA/APB.

*/

#pragma once

#include "../configuration/configuration.h"

namespace dfx {
namespace modules {


//
// SerialConfigurationBuilder 
//

class SerialConfigurationBuilder : public configuration::IBuilder {

public:

	struct {

		inout::input_node<bool> ClearAll;
		inout::input_node<configuration::AddressT> Address;
		inout::input_node<bool> WriteEnable;
		inout::input_node<configuration::DataT> WriteData;
		inout::input_node<bool> ReadRequest;
		inout::input_node<bool> ReadAcknowledge;
		inout::input_node<configuration::DataT> ReadData;

	} Inputs;

	struct {

		inout::output_node<bool> ReadAcknowledge;
		inout::output_node<dynfix> ReadData;

	} Outputs;

private:

	struct Level {

		node<bool> ClearAll;
		node<dynfix> Address;
		node<bool> WriteEnable;
		node<dynfix> WriteData;
		node<bool> ReadRequest;
		node<bool> ReadAcknowledge;
		node<dynfix> ReadData;

		dfx::HierarchyLevel *hierarchyLevel;
		int baseAddress;
	};

	configuration::Namespace &configNamespace;

	int globalNextAddress;
	int &nextAddress;

	bool isFinal;

	Level current;
	std::forward_list<Level> stack;

	bool locked;
	bus<dynfix> readBackWires;

public:

	SerialConfigurationBuilder(configuration::Namespace &theNamespace);
	SerialConfigurationBuilder(configuration::Namespace &theNamespace, int &masterNextAddress, Level &forkFrom);

	SerialConfigurationBuilder() = delete;
	SerialConfigurationBuilder(SerialConfigurationBuilder const &) = delete;
	void operator=(SerialConfigurationBuilder const &) = delete;

	void Finalise();

	SerialConfigurationBuilder &Fork();
	void Join(SerialConfigurationBuilder &from);

	node<dynfix> AddRegister(bool isSigned, int wordWidth, int fraction, std::string const &tag) override;
	void AddObserver(node<dynfix> const &source, std::string const &tag) override;
	configuration::Range AddRange(bool isSigned, int wordWidth, int fraction, int length, std::string const &tag) override;

private:

	std::list<SerialConfigurationBuilder> forkedBuilders;

	void Break(int extraPipelining) override;
	void Merge(int extraPipelining) override;

private:

	void Build();

	configuration::Namespace &GetNamespace() override;
	int GetCurrentAddress() override;

};


//
// SerialConfigurationController
//

class SerialConfigurationController : public configuration::IController {

public:

	struct {

		inout::input_node<bool> ReadAcknowledge;
		inout::input_node<configuration::DataT> ReadData;

	} Inputs;

	struct {

		inout::output_node<bool> ClearAll;
		inout::output_node<dynfix> Address;
		inout::output_node<bool> WriteEnable;
		inout::output_node<dynfix> WriteData;
		inout::output_node<bool> ReadRequest;

	} Outputs;

private:

	bool const *readAcknowledge;
	std::int32_t const *readData;

	int timeOut;

	bool clearAll;
	int address;
	bool writeEnable;
	std::int32_t writeData;
	bool readRequest;

	dfx::Simulator *simulator;

public:

	SerialConfigurationController(int theTimeOut);
	void AttachSimulator(Simulator &theSimulator);

	void Write(int theAddress, std::uint32_t const *values, int count) override;
	void Read(int theAddress, std::uint32_t *values, int count) override;
	void ClearAll() override;
};


}
}
