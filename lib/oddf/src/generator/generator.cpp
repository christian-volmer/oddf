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

	Classes that support the generation of code (e.g., Verilog) from the
	design.

*/

#include "../global.h"

#include "generator.h"
#include "../hierarchy.h"
#include "../formatting.h"

namespace dfx {
namespace generator {

bool Instance::ChildOf(Instance const *rhs) const
{
	Instance const *current = parent;

	while (current != nullptr) {

		if (current == rhs)
			return true;

		current = current->parent;
	}

	return false;
}

std::string Instance::GetFullName() const
{
	std::string fullName = name;
	Instance const *current = parent;
	while (current != nullptr) {

		fullName = current->name + "/" + fullName;
		current = current->parent;
	}
	return fullName;
}

std::size_t Instance::GetHash() const
{
	if (!cachedHashValid) {

		cachedHashValid = true;

		if (unifiedWith)
			cachedHash = unifiedWith->GetHash();
		else {

			cachedHash = 0;

			hash_combine(cachedHash, inputPorts.size());
			hash_combine(cachedHash, outputPorts.size());

			for (auto *input : inputPorts) {

				hash_combine(cachedHash, input->outputs.size());
				hash_combine(cachedHash, input->outputs[0].name);
			}

			hash_combine(cachedHash, 0x5f81d486188add2c);

			for (auto *output : outputPorts) {

				hash_combine(cachedHash, output->inputs.size());
				hash_combine(cachedHash, output->inputs[0].name);
			}

			hash_combine(cachedHash, 0xdcb41711c1cff296);

			int counter = 0x112fd1f4;
			for (auto &ent : entities) {

				hash_combine(cachedHash, ent.name);
				hash_combine(cachedHash, ent.className);

				hash_combine(cachedHash, ent.inputs.size());
				hash_combine(cachedHash, ent.outputs.size());

				for (auto const &input : ent.inputs) {

					if (input.driver) {

						hash_combine(cachedHash, input.driver->entity->index);
						hash_combine(cachedHash, input.driver->index);
						hash_combine(cachedHash, input.driver->targets.size());
						hash_combine(cachedHash, input.driver->type.GetHash());
					}

					hash_combine(cachedHash, counter++);
				}

				hash_combine(cachedHash, ent.properties.GetHash());

				if (ent.targetModule)
					hash_combine(cachedHash, ent.targetModule->GetHash());

				hash_combine(cachedHash, counter++);
			}
		}
	}

	return cachedHash;
}

//
// Entity class
//

Entity::Entity(Instance *instance, dfx::backend::BlockBase const *block) :
	block(block),
	index((int)instance->entities.size()),
	className(block->GetClassName()),
	name(block->GetName()),
	instance(instance),
	inputs(),
	outputs(),
	properties(),
	targetModule()
{
	// Input Pins
	inputs.resize(block->GetInputPins().size(), {});
	auto inIt = block->GetInputPins().begin();
	for (int i = 0; i < (int)inputs.size(); ++i, ++inIt) {

		inputs[i].entity = this;
		inputs[i].index = i;
		inputs[i].name = (*inIt)->GetName();
	}

	// Output Pins
	outputs.resize(block->GetOutputPins().size(), {});
	auto outIt = block->GetOutputPins().cbegin();
	for (int i = 0; i < (int)outputs.size(); ++i, ++outIt) {

		outputs[i].entity = this;
		outputs[i].index = i;
		outputs[i].name = (*outIt)->GetNameAndInfo(outputs[i].groupIndex, outputs[i].busSize, outputs[i].busIndex);
		outputs[i].type = (*outIt)->GetType();
	}

	block->GetProperties(properties);
}

Entity::Entity(Instance *instance, std::string const &className, std::string const &name, std::vector<std::string> const &inputNames, std::vector<std::string> const &outputNames) :
	block(nullptr),
	index((int)instance->entities.size()),
	className(className),
	name(name),
	instance(instance),
	inputs(),
	outputs(),
	properties(),
	targetModule()
{
	// Input Pins
	inputs.resize(inputNames.size(), {});
	auto inNamesIt = inputNames.begin();
	for (int i = 0; i < (int)inputs.size(); ++i, ++inNamesIt) {

		inputs[i].entity = this;
		inputs[i].index = i;
		inputs[i].name = *inNamesIt;
	}

	// Output Pins
	outputs.resize(outputNames.size(), {});
	auto outNamesIt = outputNames.cbegin();
	for (int i = 0; i < (int)outputs.size(); ++i, ++outNamesIt) {

		outputs[i].entity = this;
		outputs[i].index = i;
		outputs[i].name = *outNamesIt;
		outputs[i].groupIndex = i;
	}
}

Entity::Entity(Instance *instance, std::string const &className, std::string const &name, int numberOfInputs, int numberOfOutputs) :
	block(nullptr),
	index((int)instance->entities.size()),
	className(className),
	name(name),
	instance(instance),
	inputs(),
	outputs(),
	properties(),
	targetModule()
{
	// Input Pins
	inputs.resize(numberOfInputs, {});
	for (int i = 0; i < (int)inputs.size(); ++i) {

		inputs[i].entity = this;
		inputs[i].index = i;
	}

	// Output Pins
	outputs.resize(numberOfOutputs, {});
	for (int i = 0; i < (int)outputs.size(); ++i) {

		outputs[i].entity = this;
		outputs[i].index = i;
		outputs[i].groupIndex = i;
	}
}

bool Entity::IsConsistent() const
{
	// Check inputs
	for (int i = 0; i < (int)inputs.size(); ++i) {

		if (inputs[i].entity != this)
			return false;

		if (inputs[i].index != i)
			return false;

		// inputs must be driven from entities of the same instance
		if (inputs[i].driver != nullptr && inputs[i].driver->entity->instance != instance)
			return false;
	}

	// Check outputs
	if (outputs.size() >= 1) {

		if (outputs[0].entity != this)
			return false;

		if (outputs[0].index != 0)
			return false;

		if (outputs[0].groupIndex != 0)
			return false;

		if (outputs[0].busIndex != 0)
			return false;

		if (!(outputs[0].busSize == -1 || outputs[0].busSize > 0))
			return false;

		for (int i = 1; i < (int)outputs.size(); ++i) {

			if (outputs[i].entity != this)
				return false;

			if (outputs[i].index != i)
				return false;

			if (outputs[i].groupIndex == outputs[i - 1].groupIndex + 1) {

				if (outputs[i].busIndex != 0)
					return false;

				if (!(outputs[i].busSize == -1 || outputs[i].busSize > 0))
					return false;
			}
			else if (outputs[i].groupIndex == outputs[i - 1].groupIndex) {

				// Bus index must increase in steps of one.
				if (outputs[i].busIndex != outputs[i - 1].busIndex + 1)
					return false;

				// Bus size must be consistent
				if (outputs[i].busSize != outputs[i - 1].busSize)
					return false;

				// Bus element index must be within size of bus.
				if (outputs[i].busIndex >= outputs[i].busSize)
					return false;

				// Name must be the same for all bus elements
				if (outputs[i].name != outputs[i - 1].name)
					return false;

				// Type must be the same
				if (outputs[i].type != outputs[i - 1].type)
					return false;
			}
			else
				return false;
		}
	}

	return true;
}


//
// Generator::Configuration
//

Generator::Configuration::Configuration() :
	globalBussification(true),
	moduleBussification()
{
}


//
// Generator class
//

Generator::Generator(Design const &design, std::basic_ostream<char> &os, Configuration const &theConfiguration /* = Configuration() */) :
	configuration(theConfiguration)
{
	os << std::endl << " --- Generator --- " << std::endl << std::endl;

	for (auto &block : design.GetBlocks())
		block->Simplify();

	MapEntities(design, os);
	MapConnections(os);
	//ReportUnconnectedOutputs(os);
	PlacePorts(os);
	NamePorts(os);
	IdentifyInstances(os);
	GenerateModules(os);
	CheckConsistency(os);
}

Instance *Generator::ToInstance(dfx::HierarchyLevel const *hierarchyLevel)
{
	if (hierarchyLevel == nullptr)
		return nullptr;

	Instance *&instance = instance_mapping[hierarchyLevel];

	if (instance == nullptr) {

		instances.emplace_back(hierarchyLevel->GetSequenceNumber(), hierarchyLevel->GetName(), hierarchyLevel->GetModuleName(), ToInstance(hierarchyLevel->GetParent()));
		instance = &instances.back();
	}

	return instance;
}

void Generator::MakeConnection(Entity::Output &output, Entity::Input &input)
{
	if (input.driver != nullptr)
		throw dfx::design_error("MakeConnection: input already has a driver.");

	input.driver = &output;
	output.targets.push_back(&input);
}

void Generator::BreakConnection(Entity::Input &input)
{
	if (!input.driver)
		throw dfx::design_error("BreakConnection: input is not connected.");

	input.driver->targets.remove(&input);
	input.driver = nullptr;
}


void Generator::MapEntities(dfx::Design const &design, std::basic_ostream<char> &os)
{
	os << "Mapping entities... " << std::endl;

	auto const &blocks = design.GetBlocks();

	for (auto const &uniqueBlock : blocks) {

		auto block = uniqueBlock.get();
		if (block->CanRemove()) {

			if (!block->IsTemporary() && (block->GetClassName() != "identity"))
				design_info(block->GetFullName() + ": removing block from code generation.");
			continue;
		}

		Instance *instance = ToInstance(block->GetHierarchyLevel());

		instance->entities.emplace_back(instance, block);
		Entity &entity = instance->entities.back();
		entity_mapping[block] = &entity;
	}

	os << std::endl;
	os << " Number of entities      : " << entity_mapping.size() << std::endl;
	os << " Number of instances     : " << instance_mapping.size() << std::endl;
	os << std::endl;

	instance_mapping.clear();
}

void Generator::MapConnections(std::basic_ostream<char> &os)
{
	os << "Mapping connections... " << std::endl;

	int connectionCount = 0;
	for (auto &instance : instances) {

		for (auto &ent : instance.entities) {

			auto const *block = ent.block;

			auto const &blockInputs = block->GetInputPins();
			int numberOfInputs = (int)ent.inputs.size();

			for (int i = 0; i < numberOfInputs; ++i) {

				auto const *drivingPin = blockInputs[i]->GetDrivingPin();
				auto const *drivingBlock = drivingPin->GetOwner();
				Entity *drivingEntity = entity_mapping[drivingBlock];

				int driverIndex = (int)std::distance(
					drivingBlock->GetOutputPins().begin(),
					std::find(drivingBlock->GetOutputPins().begin(), drivingBlock->GetOutputPins().end(), drivingPin));

				auto &drivingOutput = drivingEntity->outputs[driverIndex];

				MakeConnection(drivingOutput, ent.inputs[i]);

				++connectionCount;
			}
		}
	}

	os << std::endl;
	os << " Number of connections   : " << connectionCount << std::endl;
	os << std::endl;

	entity_mapping.clear();
}

void Generator::ReportUnconnectedOutputs(std::basic_ostream<char> &os)
{
	os << "Checking for unconnected outputs... " << std::endl;

	for (auto &instance : instances) {

		for (auto &ent : instance.entities) {

			for (auto const &output : ent.outputs) {

				if (output.targets.empty())
					os << instance.GetFullName() << "/" << ent.name << "." << output.name << std::endl;
			}
		}
	}
}

void Generator::CheckConsistency(std::basic_ostream<char> &os)
{
	os << "Checking consistency... " << std::endl;

	for (auto const &instance : instances)
		for (auto const &ent : instance.entities)
			if (!ent.IsConsistent())
				throw design_error("Internal error: entity '" + instance.GetFullName() + "/" + ent.name + "' failed consistency checks.");
}

}
}
