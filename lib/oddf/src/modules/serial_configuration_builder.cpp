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

	Implementation of the configuration::IBuilder interfaces. Provides
	access a serial interface similar to AMBA/APB.

*/

#include "../global.h"
#include "serial_configuration.h"

namespace dfx {
namespace modules {

SerialConfigurationBuilder::SerialConfigurationBuilder(configuration::Namespace &theNamespace) :
	configNamespace(theNamespace),
	globalNextAddress(0),
	nextAddress(globalNextAddress),
	isFinal(false),
	current(),
	stack(),
	locked(true),
	readBackWires()
{
	namespace b = blocks;

	current.ClearAll = Inputs.ClearAll;
	current.Address = Inputs.Address;
	current.WriteEnable = Inputs.WriteEnable;
	current.WriteData = Inputs.WriteData;
	current.ReadRequest = Inputs.ReadRequest;
	current.ReadAcknowledge = Inputs.ReadAcknowledge;
	current.ReadData = Inputs.ReadData;

	current.baseAddress = 0;
	current.hierarchyLevel = nullptr;
}

SerialConfigurationBuilder::SerialConfigurationBuilder(configuration::Namespace &theNamespace, int &masterNextAddress, Level &forkFrom) :
	configNamespace(theNamespace),
	globalNextAddress(-1),
	nextAddress(masterNextAddress),
	isFinal(false),
	current(),
	stack(),
	locked(true),
	readBackWires()
{
	namespace b = blocks;

	current.ClearAll = forkFrom.ClearAll;
	current.Address = forkFrom.Address;
	current.WriteEnable = forkFrom.WriteEnable;
	current.WriteData = forkFrom.WriteData;
	current.ReadRequest = forkFrom.ReadRequest;
	current.ReadAcknowledge = b::Constant(false);
	current.ReadData = b::Constant<configuration::DataT>(0);

	current.baseAddress = forkFrom.baseAddress;
	current.hierarchyLevel = forkFrom.hierarchyLevel;
}


void SerialConfigurationBuilder::Finalise()
{
	if (!stack.empty())
		throw design_error("SerialConfigurationBuilder::Finalise(): cannot finalise because there are open Section objects.");

	isFinal = true;
}

void SerialConfigurationBuilder::Build()
{
	namespace b = blocks;

	assert(!isFinal);

	if (Design::HasCustomDefaultEnable())
		throw design_error("SerialConfigurationBuilder::Build(): cannot be used inside DFX_ENABLED_SCOPE.");

	ScopedGotoHierarchyLevel _localHierarchy(current.hierarchyLevel);

	locked = true;
	if (readBackWires.width() > 0) {

		auto addressValid = current.Address < readBackWires.width();

		current.ReadData = b::Select(b::ReinterpretCast<configuration::DataT>(readBackWires), b::Decide(addressValid, current.Address, 0));
		current.ReadAcknowledge = addressValid && current.ReadRequest;

		current.ReadData = b::Delay(current.ReadData, "Config_LocalReadData");
		current.ReadAcknowledge = b::Delay(current.ReadAcknowledge, "Config_LocalReadAcknowledge");

		readBackWires.clear();
	}
	else {

		current.ReadData = b::Constant<configuration::DataT>(0);
		current.ReadAcknowledge = b::Constant(false);
	}
}

void SerialConfigurationBuilder::Break(int extraPipelining)
{
	namespace b = blocks;

	assert(!isFinal);

	if (Design::HasCustomDefaultEnable())
		throw design_error("SerialConfigurationBuilder::Break(): cannot be used inside DFX_ENABLED_SCOPE.");

	if (!locked)
		Build();

	stack.push_front(current);

	node<dynfix> baseAddressNode;
	{
		ScopedGotoHierarchyLevel _localHierarchy(Design::GetCurrent().GetHierarchy().GetCurrentLevel()->GetParent());
		baseAddressNode = b::Constant<ufix<configuration::AddressWidth>>(nextAddress - current.baseAddress);
	}

	DFX_INPUT_NAME(baseAddressNode, "Config_BaseAddress");
	DFX_INPUT_NAME(current.ClearAll, "Config_ClearAll");
	DFX_INPUT_NAME(current.Address, "Config_Address");
	DFX_INPUT_NAME(current.WriteEnable, "Config_WriteEnable");
	DFX_INPUT_NAME(current.WriteData, "Config_WriteData");
	DFX_INPUT_NAME(current.ReadRequest, "Config_ReadRequest");
	DFX_INPUT_NAME(current.ReadAcknowledge, "Config_ReadAcknowledge");
	DFX_INPUT_NAME(current.ReadData, "Config_ReadData");

	node<bool> addressValid = current.Address >= baseAddressNode;

	current.ClearAll = current.ClearAll;
	current.Address = b::Decide(addressValid, b::FloorCast(current.Address, current.Address - baseAddressNode), 0);
	current.WriteEnable = addressValid && current.WriteEnable;
	current.WriteData = b::Decide(addressValid, current.WriteData, 0);
	current.ReadRequest = addressValid && current.ReadRequest;
	current.baseAddress = nextAddress;
	current.hierarchyLevel = Design::GetCurrent().GetHierarchy().GetCurrentLevel();

	int pipelining = 1 + extraPipelining;
	while (pipelining-- > 0) {

		current.ClearAll = b::Delay(current.ClearAll, "Config_LocalClearAll");
		current.Address = b::Delay(current.Address, "Config_LocalAddress");
		current.WriteEnable = b::Delay(current.WriteEnable, "Config_LocalWriteEnable");
		current.WriteData = b::Delay(current.WriteData, "Config_LocalWriteData");
		current.ReadRequest = b::Delay(current.ReadRequest, "Config_LocalReadRequest");
	}

	current.ReadAcknowledge = node<bool>();
	current.ReadData = node<dynfix>();

	locked = false;
}

void SerialConfigurationBuilder::Merge(int extraPipelining)
{
	namespace b = blocks;

	/*

	// We add a dummy register after the very last register in the main config bus.
	// This is to avoid SPI read time-out errors when reading the last register due to 
	// read-ahead feature.
	if ((std::next(stack.begin()) == stack.end()) && (globalNextAddress != -1)) {

		if (locked) {

			Break(0);
			AddObserver(b::Constant<ufix<1>>(0), "dummy");
			Merge(0);
		}
		else
			AddObserver(b::Constant<ufix<1>>(0), "dummy");
	}

	WARNING: breaks code-generation, creates duplicate input 'in_Config_BaseAddress'

	*/

	assert(!isFinal);

	if (Design::HasCustomDefaultEnable())
		throw design_error("SerialConfigurationBuilder::Merge(): cannot be used inside DFX_ENABLED_SCOPE.");

	if (!locked)
		Build();

	b::Terminate(current.ClearAll);
	b::Terminate(current.Address);
	b::Terminate(current.WriteEnable);
	b::Terminate(current.WriteData);
	b::Terminate(current.ReadRequest);

	current.ClearAll = stack.front().ClearAll;
	current.Address = stack.front().Address;
	current.WriteEnable = stack.front().WriteEnable;
	current.WriteData = stack.front().WriteData;
	current.ReadRequest = stack.front().ReadRequest;
	current.ReadData = b::Decide(
		stack.front().ReadAcknowledge, stack.front().ReadData,
		current.ReadAcknowledge, current.ReadData,
		0);

	current.ReadAcknowledge = stack.front().ReadAcknowledge || current.ReadAcknowledge;
	current.baseAddress = stack.front().baseAddress;

	stack.pop_front();

	int pipelining = 1 + extraPipelining;
	while (pipelining-- > 0) {

		current.ClearAll = b::Delay(current.ClearAll, "Config_MergedClearAll");
		current.Address = b::Delay(current.Address, "Config_MergedAddress");
		current.WriteEnable = b::Delay(current.WriteEnable, "Config_MergedWriteEnable");
		current.WriteData = b::Delay(current.WriteData, "Config_MergedWriteData");
		current.ReadRequest = b::Delay(current.ReadRequest, "Config_MergedReadRequest");
		current.ReadAcknowledge = b::Delay(current.ReadAcknowledge, "Config_MergedReadAcknowledge");
		current.ReadData = b::Delay(current.ReadData, "Config_MergedReadData");
	}

	Outputs.ReadAcknowledge = current.ReadAcknowledge;
	Outputs.ReadData = current.ReadData;

	DFX_OUTPUT_NAME(current.ClearAll, "Config_ClearAll");
	DFX_OUTPUT_NAME(current.Address, "Config_Address");
	DFX_OUTPUT_NAME(current.WriteEnable, "Config_WriteEnable");
	DFX_OUTPUT_NAME(current.WriteData, "Config_WriteData");
	DFX_OUTPUT_NAME(current.ReadRequest, "Config_ReadRequest");
	DFX_OUTPUT_NAME(current.ReadAcknowledge, "Config_ReadAcknowledge");
	DFX_OUTPUT_NAME(current.ReadData, "Config_ReadData");
}

SerialConfigurationBuilder &SerialConfigurationBuilder::Fork()
{
	assert(!isFinal);

	if (Design::HasCustomDefaultEnable())
		throw design_error("SerialConfigurationBuilder::Fork(): cannot be used inside DFX_ENABLED_SCOPE.");

	if (!locked)
		Build();

	forkedBuilders.emplace_back(configNamespace, globalNextAddress, current);
	return forkedBuilders.back();
}

void SerialConfigurationBuilder::Join(SerialConfigurationBuilder &from)
{
	namespace b = blocks;

	assert(!isFinal);
	assert(from.globalNextAddress == -1);

	if (Design::HasCustomDefaultEnable())
		throw design_error("SerialConfigurationBuilder::Join(): cannot be used inside DFX_ENABLED_SCOPE.");

	from.Finalise();

	if (!locked)
		Build();

	current.ClearAll = b::Delay(current.ClearAll);
	b::Terminate(from.current.ClearAll);

	current.Address = b::Delay(current.Address);
	b::Terminate(from.current.Address);

	current.WriteEnable = b::Delay(current.WriteEnable);
	b::Terminate(from.current.WriteEnable);

	current.WriteData = b::Delay(current.WriteData);
	b::Terminate(from.current.WriteData);

	current.ReadRequest = b::Delay(current.ReadRequest);
	b::Terminate(from.current.ReadRequest);

	current.ReadData = b::Delay(b::Decide(
		current.ReadAcknowledge, current.ReadData,
		from.current.ReadAcknowledge, from.current.ReadData,
		0));

	current.ReadAcknowledge = b::Delay(from.current.ReadAcknowledge || current.ReadAcknowledge);
}

configuration::Namespace &SerialConfigurationBuilder::GetNamespace()
{
	return configNamespace;
}

int SerialConfigurationBuilder::GetCurrentAddress()
{
	return nextAddress;
}

node<dynfix> SerialConfigurationBuilder::AddRegister(bool isSigned, int wordWidth, int fraction, std::string const &tag)
{
	namespace b = blocks;

	assert(!isFinal);
	assert(locked == false);

	if (Design::HasCustomDefaultEnable())
		throw design_error("SerialConfigurationBuilder::AddRegister(): cannot be used inside DFX_ENABLED_SCOPE.");

	dynfix zero = dynfix(isSigned, wordWidth, fraction);
	forward_node<dynfix> value(zero);

	int localAddress = nextAddress - current.baseAddress;

	if (wordWidth <= configuration::DataWidth) {

		value <<= b::Delay(b::Decide(
			current.ClearAll, zero,
			current.WriteEnable && current.Address == localAddress, b::ReinterpretCast(value, current.WriteData), 
			value), tag);
		readBackWires.append(value);
		++nextAddress;
	}
	else if (wordWidth <= 2 * configuration::DataWidth) {

		// high order bits come at current address and are stored in a shadow register
		dynfix highZero = dynfix(isSigned, wordWidth - configuration::DataWidth, fraction - configuration::DataWidth);

		forward_node<dynfix> highValue(highZero);
		highValue <<= b::Delay(b::Decide(
			current.ClearAll, highZero,
			current.WriteEnable && current.Address == localAddress, b::ReinterpretCast(highValue, current.WriteData),
			highValue));

		// low order bits come at current address + 1. Higher order bits are copied from shadow 
		// register when lower order bits are written to.
		value <<= b::Delay(b::Decide(
			current.ClearAll, zero,
			current.WriteEnable && current.Address == localAddress + 1, b::FloorCast(value, b::ReinterpretCast(value, current.WriteData) + highValue),
			value), tag);

		readBackWires.append(b::FloorCast(highValue, value));
		readBackWires.append(value);
		nextAddress += 2;
	}
	else
		throw design_error("SerialConfigurationBuilder::AddRegister(): word widths greater than 2 * 'DataWidth' bit are not supported.");

	return value;
}

void SerialConfigurationBuilder::AddObserver(node<dynfix> const &source, std::string const &tag)
{
	namespace b = blocks;

	assert(!isFinal);
	assert(locked == false);

	if (Design::HasCustomDefaultEnable())
		throw design_error("SerialConfigurationBuilder::AddObserver(): cannot be used inside DFX_ENABLED_SCOPE.");

	bool isSigned = source.GetType().IsSigned();
	int wordWidth = source.GetType().GetWordWidth();
	int fraction = source.GetType().GetFraction();

	auto reg = b::Delay(source, tag);

	if (wordWidth <= configuration::DataWidth) {

		readBackWires.append(reg);
		++nextAddress;
	}
	else if (wordWidth <= 2 * configuration::DataWidth) {

		dynfix highZero = dynfix(isSigned, wordWidth - configuration::DataWidth, fraction - configuration::DataWidth);

		readBackWires.append(b::FloorCast(highZero, reg));
		readBackWires.append(reg);
		nextAddress += 2;
	}
	else
		throw design_error("SerialConfigurationBuilder::AddObserver(): word widths greater than 2 * 'DataWidth' bit are not supported.");
}

configuration::Range SerialConfigurationBuilder::AddRange(bool isSigned, int wordWidth, int fraction, int length, std::string const &tag)
{
	(void)tag;

	namespace b = blocks;

	assert(!isFinal);

	if (Design::HasCustomDefaultEnable())
		throw design_error("SerialConfigurationBuilder::AddRange(): cannot be used inside DFX_ENABLED_SCOPE.");

	if (wordWidth > configuration::DataWidth)
		design_error("SerialConfigurationBuilder::AddRange(): word widths greater than 'DataWidth' bit are not supported.");

	if (!locked)
		Build();

	int localAddress = nextAddress - current.baseAddress;

	configuration::Range range(isSigned, wordWidth, fraction);

	int addressWidth = 1, temp = length - 1;
	while ((temp = temp / 2) > 0) ++addressWidth;

	dynfix addressZero = dynfix(false, addressWidth, 0);
	dynfix dataZero = dynfix(isSigned, wordWidth, fraction);

	node<bool> addressValid = current.Address >= localAddress && current.Address < (localAddress + length);

	range.ClearAll = current.ClearAll;

	range.Address = b::Decide(
		addressValid, b::FloorCast(addressZero, current.Address - localAddress),
		addressZero);

	range.WriteEnable = addressValid && current.WriteEnable;

	range.WriteData = b::Decide(
		range.WriteEnable, b::ReinterpretCast(dataZero, current.WriteData),
		dataZero);

	range.ReadRequest = addressValid && current.ReadRequest;

	current.ReadData = b::Delay(b::Decide(
		current.ReadAcknowledge, current.ReadData,
		range.ReadAcknowledge, b::ReinterpretCast<configuration::DataT>(range.ReadData),
		0));

	current.ReadAcknowledge = b::Delay(current.ReadAcknowledge || range.ReadAcknowledge);

	Outputs.ReadAcknowledge = current.ReadAcknowledge;
	Outputs.ReadData = current.ReadData;

	nextAddress += length;

	return range;
}


}
}
