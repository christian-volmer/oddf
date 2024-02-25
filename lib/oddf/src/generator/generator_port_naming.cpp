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
	design. Implements the naming of module inputs and outputs.

*/

#include "../global.h"

#include "generator.h"
#include "../hierarchy.h"
#include "../formatting.h"

namespace dfx {
namespace generator {

void Generator::AssignBusInputLabels(Instance &instance, std::unordered_set<Entity *> &unassignedInputPorts, std::list<Entity *> &inputLabels, bool bussify)
{
	int newInputPortsCounter = 0;

	for (auto *labelEnt : inputLabels) {

		if (labelEnt->properties.GetInt("isBus") == 0)
			continue;

		std::size_t size = labelEnt->inputs.size();

		if (size < 1)
			continue;

		std::string name = labelEnt->inputs[0].name;

		// Collect all input_port entities belonging to this bus.
		std::vector<Entity *> inputPorts;
		inputPorts.reserve(size);

		types::TypeDescription typeDesc;
		bool allSameType = true;

		for (auto &input : labelEnt->inputs) {

			// Driver of this label is the input port we wish to rename
			auto *driver = input.driver;
			assert(driver != nullptr);
			assert(driver->type.IsKnown());

			// All nodes in the bus must share the same type.
			if (typeDesc.IsKnown()) {

				if (typeDesc != driver->type)
					allSameType = false;
			}
			else
				typeDesc = driver->type;

			auto *inputPort = driver->entity;
			assert(inputPort->instance == &instance);

			auto inputPortIt = unassignedInputPorts.find(inputPort);
			if (inputPortIt == unassignedInputPorts.end())
				break;

			assert(inputPort->className == "input_port");
			assert(inputPort->inputs.size() == 1);
			assert(inputPort->outputs.size() == 1);
			assert(inputPort->inputs[0].driver != nullptr);
			assert(inputPort->inputs[0].driver->entity->instance == instance.parent);
			assert(inputPort->inputs[0].driver->entity->className == "$input_connector");

			inputPorts.push_back(inputPort);
		}

		// Confirm that the bus is complete.
		if (inputPorts.size() != size)
			continue;

		// Place all input ports in a set to remove duplicates and to speed up membership testing.
		std::unordered_set<Entity *> inputPortsSet(inputPorts.begin(), inputPorts.end());

		// Check that there are no duplicates.
		if (inputPortsSet.size() != size)
			continue;

		if (!allSameType || !bussify) {

			// Do not create bus input but use underscores instead
			for (std::size_t i = 0; i < size; ++i)
				inputPorts[i]->outputs[0].name = name + "_" + std::to_string(i);

			continue;
		}

		// Create new input_port entity that comprises all nodes on the bus
		std::vector<std::string> emptyNames(size);
		std::vector<std::string> busNames(size, name);

		instance.entities.emplace_back(
			&instance,
			"input_port",
			string_printf("bus_input_port%d", ++newInputPortsCounter),
			emptyNames,
			busNames
			);

		Entity &busInputPort = instance.entities.back();

		// Rewire all original input_ports to this new bus port
		for (std::size_t i = 0; i < size; ++i) {

			Entity::Output *connector = inputPorts[i]->inputs[0].driver;
			std::list<Entity::Input *> targets = inputPorts[i]->outputs[0].targets;

			BreakConnection(inputPorts[i]->inputs[0]);
			for (auto *target : targets)
				BreakConnection(*target);

			MakeConnection(*connector, busInputPort.inputs[i]);
			for (auto *target : targets)
				MakeConnection(busInputPort.outputs[i], *target);

			busInputPort.outputs[i].type = typeDesc;
			busInputPort.outputs[i].busIndex = (int)i;
			busInputPort.outputs[i].busSize = (int)size;
			busInputPort.outputs[i].groupIndex = 0;

			unassignedInputPorts.erase(inputPorts[i]);
		}

		// Remove all original input_port entities that became replaced by the new bus input_port.
		instance.entities.remove_if([&](Entity &ent) {

			if ((ent.className == "input_port") && inputPortsSet.find(&ent) != inputPortsSet.end()) {

				assert(ent.inputs[0].driver == nullptr);
				assert(ent.outputs[0].targets.size() == 0);
				return true;
			}
			else
				return false;
		});
	}
}

void Generator::AssignNodeInputLabels(Instance &instance, std::unordered_set<Entity *> &unassignedInputPorts, std::list<Entity *> &inputLabels)
{
	for (auto *labelEnt : inputLabels) {

		if (labelEnt->properties.GetInt("isBus") == 1)
			continue;

		assert(labelEnt->inputs.size() == 1);

		// Driver of this label is the input port we wish to rename
		auto *driver = labelEnt->inputs[0].driver;
		assert(driver != nullptr);

		auto *inputPort = driver->entity;
		assert(inputPort->instance == &instance);

		auto inputPortIt = unassignedInputPorts.find(inputPort);
		if (inputPortIt == unassignedInputPorts.end())
			continue;

		assert(inputPort->className == "input_port");

		assert(inputPort->inputs.size() == 1);
		assert(inputPort->outputs.size() == 1);
		assert(inputPort->inputs[0].driver != nullptr);
		assert(inputPort->inputs[0].driver->entity->instance == instance.parent);
		assert(inputPort->inputs[0].driver->entity->className == "$input_connector");

		inputPort->outputs[0].name = labelEnt->inputs[0].name;
		unassignedInputPorts.erase(inputPortIt);
	}
}

void Generator::AssignBusOutputLabels(Instance &instance, std::unordered_set<Entity *> &unassignedOutputPorts, std::list<Entity *> &outputLabels, bool bussify)
{
	int newOutputPortsCounter = 0;

	for (auto *labelEnt : outputLabels) {

		if (labelEnt->properties.GetInt("isBus") == 0)
			continue;

		std::size_t size = labelEnt->inputs.size();

		if (size < 1)
			continue;

		std::string name = labelEnt->inputs[0].name;

		// Collect all output_port entities belonging to this bus.
		std::vector<Entity *> outputPorts;
		outputPorts.reserve(size);

		types::TypeDescription typeDesc;
		bool allSameType = true;

		for (auto &input : labelEnt->inputs) {

			// Find the driver of this bus input.
			auto *driver = input.driver;
			assert(driver->entity->instance == &instance);
			assert(driver->type.IsKnown());

			// All nodes in the bus must share the same type.
			if (typeDesc.IsKnown()) {

				if (typeDesc != driver->type)
					allSameType = false;
			}
			else
				typeDesc = driver->type;

			// From the set of unassigned output_port entities pick one attached to this driver.
			auto outputPortIt = std::find_if(unassignedOutputPorts.begin(), unassignedOutputPorts.end(),
											 [driver](Entity *outputPort) { return outputPort->inputs[0].driver == driver; });

			if (outputPortIt == unassignedOutputPorts.end())
				break; // The bus is not complete, we have to abort.

			Entity *outputPort = *outputPortIt;
			assert(outputPort->inputs.size() == 1);
			assert(outputPort->outputs.size() == 1);
			assert(outputPort->outputs[0].targets.size() == 1);
			assert(outputPort->outputs[0].targets.front()->entity->instance == instance.parent);
			assert(outputPort->outputs[0].targets.front()->entity->className == "$output_connector");

			outputPorts.push_back(outputPort);
		}

		// Confirm that the bus is complete.
		if (outputPorts.size() != size)
			continue;

		// Place all output ports in a set to remove duplicates and to speed up membership testing.
		std::unordered_set<Entity *> outputPortsSet(outputPorts.begin(), outputPorts.end());

		// Check that there are no duplicates.
		if (outputPortsSet.size() != size)
			continue;

		if (!allSameType || !bussify) {

			// Do not create bus input but use underscores instead
			for (std::size_t i = 0; i < size; ++i)
				outputPorts[i]->inputs[0].name = name + "_" + std::to_string(i);

			continue;
		}

		// Create new output_port entity that comprises all nodes on the bus
		std::vector<std::string> emptyNames(size);
		std::vector<std::string> busNames(size, name);

		instance.entities.emplace_back(
			&instance,
			"output_port",
			string_printf("bus_output_port%d", ++newOutputPortsCounter),
			busNames,
			emptyNames
			);

		Entity &busOutputPort = instance.entities.back();

		// Rewire all original output_ports to this new bus port
		for (std::size_t i = 0; i < size; ++i) {

			Entity::Output *driver = outputPorts[i]->inputs[0].driver;
			Entity::Input *connector = outputPorts[i]->outputs[0].targets.front();

			BreakConnection(outputPorts[i]->inputs[0]);
			BreakConnection(*connector);

			MakeConnection(*driver, busOutputPort.inputs[i]);
			MakeConnection(busOutputPort.outputs[i], *connector);

			busOutputPort.outputs[i].type = typeDesc;
			busOutputPort.outputs[i].busIndex = (int)i;
			busOutputPort.outputs[i].busSize = (int)size;
			busOutputPort.outputs[i].groupIndex = 0;

			unassignedOutputPorts.erase(outputPorts[i]);
		}

		// Remove all original output_port entities that became replaced by the new bus output_port.
		instance.entities.remove_if([&](Entity &ent) {

			if ((ent.className == "output_port") && outputPortsSet.find(&ent) != outputPortsSet.end()) {

				assert(ent.inputs[0].driver == nullptr);
				assert(ent.outputs[0].targets.size() == 0);
				return true;
			}
			else
				return false;
		});
	}
}

void Generator::AssignNodeOutputLabels(Instance &instance, std::unordered_set<Entity *> &unassignedOutputPorts, std::list<Entity *> &outputLabels)
{
	for (auto *labelEnt : outputLabels) {

		if (labelEnt->properties.GetInt("isBus") == 1)
			continue;

		assert(labelEnt->inputs.size() == 1);

		// Find the driver this label is attached to.
		auto *driver = labelEnt->inputs[0].driver;
		assert(driver->entity->instance == &instance);

		// From the set of unassigned output_port entities pick one attached to this driver.
		auto outputPortIt = std::find_if(unassignedOutputPorts.begin(), unassignedOutputPorts.end(),
										 [driver](Entity *outputPort) { return outputPort->inputs[0].driver == driver; });

		if (outputPortIt == unassignedOutputPorts.end())
			continue;

		Entity *outputPort = *outputPortIt;
		assert(outputPort->inputs.size() == 1);
		assert(outputPort->outputs.size() == 1);
		assert(outputPort->outputs[0].targets.size() == 1);
		assert(outputPort->outputs[0].targets.front()->entity->instance == instance.parent);
		assert(outputPort->outputs[0].targets.front()->entity->className == "$output_connector");

		outputPort->inputs[0].name = labelEnt->inputs[0].name;
		unassignedOutputPorts.erase(outputPortIt);
	}
}

void Generator::NamePorts(std::basic_ostream<char> &os)
{
	os << "Naming input and output ports... " << std::endl;

	for (auto &instance : instances) {

		// Set of "input_port" entities that have yet to be assigned a name.
		std::unordered_set<Entity *> unassignedInputPorts;

		// Set of "output_port" entities that have yet to be assigned a name.
		std::unordered_set<Entity *> unassignedOutputPorts;

		// List of all input labels in this instance.
		std::list<Entity *> inputLabels;

		// List of all output labels in this instance.
		std::list<Entity *> outputLabels;

		for (auto &ent : instance.entities) {

			if (ent.className == "input_port")
				unassignedInputPorts.insert(&ent);
			else if (ent.className == "output_port")
				unassignedOutputPorts.insert(&ent);
			else if (ent.className == "$label" && ent.properties.GetString("class") == "input")
				inputLabels.push_back(&ent);
			else if (ent.className == "$label" && ent.properties.GetString("class") == "output")
				outputLabels.push_back(&ent);
		}

		// Bussification for this instance?

		bool bussify = configuration.globalBussification;
		auto moduleBussificationIt = configuration.moduleBussification.find(instance.userModuleName);
		if (moduleBussificationIt != configuration.moduleBussification.end())
			bussify = moduleBussificationIt->second;


		AssignBusInputLabels(instance, unassignedInputPorts, inputLabels, bussify);
		AssignBusOutputLabels(instance, unassignedOutputPorts, outputLabels, bussify);

		AssignNodeInputLabels(instance, unassignedInputPorts, inputLabels);
		AssignNodeOutputLabels(instance, unassignedOutputPorts, outputLabels);

		// Recollect input and output ports and name unlabeled ports
		int unnamedInputsCount = 0 , unnamedOutputsCount = 0;
		instance.inputPorts.clear();
		instance.outputPorts.clear();
		for (auto &entity : instance.entities) {

			if (entity.className == "input_port") {

				if (entity.outputs[0].name.empty()) {

					std::string label = "Unnamed" + std::to_string(unnamedInputsCount++);

					for (auto &output : entity.outputs)
						output.name = label;
				}

				instance.inputPorts.push_back(&entity);
			}

			if (entity.className == "output_port") {

				if (entity.inputs[0].name.empty()) {

					std::string label = "Unnamed" + std::to_string(unnamedOutputsCount++);

					for (auto &input : entity.inputs)
						input.name = label;
				}

				instance.outputPorts.push_back(&entity);
			}
		}
	}
}

}
}
