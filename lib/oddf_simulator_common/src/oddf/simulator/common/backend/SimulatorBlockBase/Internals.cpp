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

#include "Internals.h"

#include <oddf/utility/MakeContainerView.h>

#include <oddf/Exception.h>

#include <vector>
#include <utility>
#include <cassert>

namespace oddf::simulator::common::backend {

SimulatorBlockBase::Internals::Internals(SimulatorBlockBase &owningBlock, design::blocks::backend::IDesignBlock const &designBlock) :
	m_component(nullptr),
	m_visiting(false),
	m_removed(false),
	m_designBlockReference(&designBlock),
	m_inputs(),
	m_outputs()
{

	/*
	    TODO

	    The following code goes though the outputs of the design block to copy
	    their `NodeType` to a temporary vector that is then converted to a
	    `ListView` which can then be passed to `InitialiseInputsAndOutputs`.

	    Seems cumbersome. Can be implement some sort of 'transform' function
	    for `ListView`, where elements become transformed in place?

	    Would this be possible despite InitialiseInputsAndOutputs requiring a
	    `ListView` of const references?

	    cf.
	      https://stackoverflow.com/questions/11560339/returning-temporary-object-and-binding-to-const-reference
	      https://herbsutter.com/2008/01/01/gotw-88-a-candidate-for-the-most-important-const/

	*/

	auto designBlockOutputsList = designBlock.GetOutputsList();

	std::vector<design::NodeType> designBlockOutputNodeTypes;
	designBlockOutputNodeTypes.reserve(designBlockOutputsList->GetSize());

	auto outputsEnum = designBlockOutputsList->GetEnumerator();
	while (outputsEnum->MoveNext())
		designBlockOutputNodeTypes.push_back(outputsEnum->GetCurrent().GetNodeType());

	InitialiseInputsAndOutputs(
		owningBlock,
		designBlock.GetInputsList()->GetSize(),
		*utility::MakeContainerView(std::as_const(designBlockOutputNodeTypes)));
}

SimulatorBlockBase::Internals::Internals(SimulatorBlockBase &owningBlock, design::blocks::backend::IDesignBlock const *designBlock, size_t numberOfInputs, std::initializer_list<design::NodeType> outputNodeTypes) :
	m_component(nullptr),
	m_visiting(false),
	m_removed(false),
	m_designBlockReference(designBlock),
	m_inputs(),
	m_outputs()
{
	InitialiseInputsAndOutputs(owningBlock, numberOfInputs, *utility::MakeContainerView(outputNodeTypes));
}

void SimulatorBlockBase::Internals::InitialiseInputsAndOutputs(SimulatorBlockBase &owningBlock, size_t numberOfInputs,
	ICollectionView<design::NodeType const &> const &outputNodeTypes)
{
	m_inputs.reserve(numberOfInputs);
	for (size_t i = 0; i < numberOfInputs; ++i)
		m_inputs.emplace_back(owningBlock, i);

	auto numberOfOutputs = outputNodeTypes.GetSize();
	m_outputs.reserve(numberOfOutputs);

	auto outputNodeTypesEnum = outputNodeTypes.GetEnumerator();

	for (size_t i = 0; i < numberOfOutputs; ++i) {

		outputNodeTypesEnum->MoveNext();
		m_outputs.emplace_back(owningBlock, outputNodeTypesEnum->GetCurrent(), i);
	}
}

void SimulatorBlockBase::Internals::MapConnections(ISimulatorBlockMapping const &blockMapping)
{
	if (!m_designBlockReference)
		throw oddf::Exception(oddf::ExceptionCode::IllegalMethodCall);

	auto designInputEnumerator = m_designBlockReference->GetInputsList()->GetEnumerator();

	for (auto &simInput : m_inputs) {

		designInputEnumerator->MoveNext();
		auto &designInput = designInputEnumerator->GetCurrent();

		if (designInput.IsConnected()) {

			auto &designDriver = designInput.GetDriver();
			auto &designDrivingBlock = designDriver.GetOwningBlock();
			auto driverIndex = designDriver.GetIndex();

			auto *simDrivingBlock = blockMapping.DesignBlockToSimulatorBlock(designDrivingBlock);
			if (simDrivingBlock) {

				auto &simDriver = simDrivingBlock->m_internals->m_outputs[driverIndex];
				simInput.ConnectTo(simDriver);
			}
			else {

				throw Exception(ExceptionCode::Fail, "Block '" + m_designBlockReference->GetPath().ToString() + "': driving block '" + designDrivingBlock.GetPath().ToString() + "' not found.");
			}
		}
	}

	assert(!designInputEnumerator->MoveNext());
}

} // namespace oddf::simulator::common::backend
