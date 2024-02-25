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
	design. Implements the identification of modules and their instances.

*/

#include "../global.h"

#include "generator.h"
#include "../hierarchy.h"
#include "../formatting.h"

namespace dfx {
namespace generator {

void Generator::IdentifyInstances(std::basic_ostream<char> &os)
{
	os << "Identifying instances... " << std::endl;

	for (auto &instance : instances) {

		struct Connectivity {

			std::list<Entity *> inputConnectors;
			std::list<Entity *> outputConnectors;
		};

		struct InstanceSequenceCompare {

			bool operator()(Instance const *lhs, Instance const *rhs) const
			{
				return lhs->sequenceNumber < rhs->sequenceNumber;
			}
		};

		std::map<Instance *, Connectivity, InstanceSequenceCompare> instantiations;

		for (auto &ent : instance.entities) {

			if (ent.className == "$input_connector")
				instantiations[ent.outputs[0].targets.front()->entity->instance].inputConnectors.push_back(&ent);
			else if (ent.className == "$output_connector")
				instantiations[ent.inputs[0].driver->entity->instance].outputConnectors.push_back(&ent);
		}

		for (auto &element : instantiations) {

			int portIndex = 0;
			int groupIndex = 0;

			Instance *module = element.first;
			Connectivity &connectivity = element.second;

			assert(connectivity.inputConnectors.size() == std::accumulate(module->inputPorts.begin(), module->inputPorts.end(), std::size_t(0), [](std::size_t count, Entity const *outputPort) { return count + outputPort->inputs.size(); }));
			assert(connectivity.outputConnectors.size() == std::accumulate(module->outputPorts.begin(), module->outputPorts.end(), std::size_t(0), [](std::size_t count, Entity const *outputPort) { return count + outputPort->inputs.size(); }));

			instance.entities.emplace_back(
				&instance,
				"instance",
				module->name,
				std::vector<std::string>(connectivity.inputConnectors.size()),
				std::vector<std::string>(connectivity.outputConnectors.size())
				);

			Entity &instanceEntity = instance.entities.back();
			instanceEntity.targetModule = module;

			// Rewire $input_connector to their respective input ports of the newly created 'instance' entity.
			portIndex = 0;
			for (auto &inputPort : module->inputPorts) {

				for (int i = 0; i < (int)inputPort->inputs.size(); ++i) {

					auto &portOutput = inputPort->outputs[i];
					auto &portInput = inputPort->inputs[i];

					Entity *inputConnector = portInput.driver->entity;
					assert(inputConnector->instance == &instance);
					assert(inputConnector->inputs.size() == 1);
					assert(inputConnector->outputs.size() == 1);
					assert(inputConnector->outputs[0].targets.size() == 1);

					instanceEntity.inputs[portIndex].name = "In_" + portOutput.name;

					auto *source = inputConnector->inputs[0].driver;

					// Completely disconnect the '$input_connector' and leave the input of the corresponding 'input_port' unconnected
					BreakConnection(inputConnector->inputs[0]);
					BreakConnection(portInput);

					// Reconnect the original driver of the '$input_connector' to the corresponding input of the 'instance' entity.
					MakeConnection(*source, instanceEntity.inputs[portIndex]);

					++portIndex;
				}
			}

			// Rewire $output_connector to their respective output ports of the newly created 'instance' entity.
			portIndex = 0;
			groupIndex = 0;
			for (auto &outputPort : module->outputPorts) {

				for (int i = 0; i < (int)outputPort->outputs.size(); ++i) {

					auto &portOutput = outputPort->outputs[i];
					auto &portInput = outputPort->inputs[i];

					assert(portOutput.targets.size() == 1);

					Entity *outputConnector = portOutput.targets.front()->entity;
					assert(outputConnector->instance == &instance);
					assert(outputConnector->inputs.size() == 1);
					assert(outputConnector->outputs.size() == 1);

					instanceEntity.outputs[portIndex].name = "Out_" + portInput.name;
					instanceEntity.outputs[portIndex].type = portOutput.type;
					instanceEntity.outputs[portIndex].busSize = portOutput.busSize;
					instanceEntity.outputs[portIndex].busIndex = portOutput.busIndex;
					instanceEntity.outputs[portIndex].groupIndex = groupIndex;

					std::list<Entity::Input *> targets = outputConnector->outputs[0].targets;

					// Completely disconnect the '$output_connector' and leave the input of the corresponding 'input_port' unconnected
					for (auto *target : targets)
						BreakConnection(*target);
					BreakConnection(outputConnector->inputs[0]);

					// Reconnect the original targets of the '$output_connector' to the corresponding output of the 'instance' entity.
					for (auto *target : targets)
						MakeConnection(instanceEntity.outputs[portIndex], *target);

					++portIndex;
				}

				++groupIndex;
			}
		}

		// Remove all '$input_connector' and '$output_connector' entities from their instance
		// and double-check that all '$input_connector' and '$output_connector' have been completely disconnected 

		instance.entities.remove_if([](Entity const &ent) {

			if ((ent.className == "$input_connector") || (ent.className == "$output_connector")) {

				assert(ent.inputs[0].driver == nullptr);
				assert(ent.outputs[0].targets.size() == 0);
				return true;
			}
			else
				return false;
		});
	}
}

std::size_t Generator::InstanceHash(Instance const &instance)
{
	return instance.GetHash();
}

bool Generator::InstancesEqual(Instance const *instance1, Instance const *instance2)
{
	if (instance1->entities.size() != instance2->entities.size())
		return false;

	if (instance1->inputPorts.size() != instance2->inputPorts.size())
		return false;

	if (instance1->outputPorts.size() != instance2->outputPorts.size())
		return false;

	// Do the quick checks first
	{
		auto it1 = instance1->entities.cbegin();
		auto it2 = instance2->entities.cbegin();
		auto end = instance1->entities.cend();

		while (it1 != end) {

			if (it1->className != it2->className)
				return false;

			if (it1->inputs.size() != it2->inputs.size())
				return false;

			if (it1->outputs.size() != it2->outputs.size())
				return false;

			if (it1->targetModule != it2->targetModule)
				return false;

			++it1;
			++it2;
		}
	}

	{
		auto it1 = instance1->entities.cbegin();
		auto it2 = instance2->entities.cbegin();
		auto end = instance1->entities.cend();

		while (it1 != end) {

			// Compare inputs
			if (!std::equal(it1->inputs.cbegin(), it1->inputs.cend(), it2->inputs.cbegin(),
				[](Entity::Input const &input1, Entity::Input const &input2) {

				if (input1.driver == nullptr) {

					if (input2.driver == nullptr)
						return true;
					else
						return false;
				}

				if (input1.driver->entity->index != input2.driver->entity->index)
					return false;

				if (input1.driver->index != input2.driver->index)
					return false;

				if (input1.driver->targets.size() != input2.driver->targets.size())
					return false;

				if (input1.driver->type != input2.driver->type)
					return false;

				return true;
			}))
				return false;

			// TODO: do we need to check the outputs as well? I think they are checked implicitely by the input check.

			if (it1->properties != it2->properties)
				return false;

			++it1;
			++it2;
		}
	}

	// compare names of inputs
	for (int i = 0; i < (int)instance1->inputPorts.size(); ++i)
		for (int j = 0; j < (int)instance1->inputPorts[i]->outputs.size(); ++j)
			if (instance1->inputPorts[i]->outputs[j].name != instance2->inputPorts[i]->outputs[j].name)
				return false;

	// compare names of outputs
	for (int i = 0; i < (int)instance1->outputPorts.size(); ++i)
		for (int j = 0; j < (int)instance1->outputPorts[i]->inputs.size(); ++j)
			if (instance1->outputPorts[i]->inputs[j].name != instance2->outputPorts[i]->inputs[j].name)
				return false;

	return true;
}

void Generator::UnifyInstances(Instance *referenceInstance, Instance *instance, std::list<Entity *> &instanceEntities)
{
	for (auto &entity : instanceEntities)
		if (entity->targetModule == instance)
			entity->targetModule = referenceInstance;

	instance->unifiedWith = referenceInstance;
}

void Generator::GenerateModules(std::basic_ostream<char> &os)
{
	os << "Generating modules... " << std::endl;

	// Collect all entities in the design with className 'instance' for fast access
	std::list<Entity *> instanceEntities;
	for (auto &instance : instances) {

		for (auto &entity : instance.entities)
			if (entity.className == "instance")
				instanceEntities.push_back(&entity);
	}

	// Identify modules with identical content and unify. Repeat this until all modules are distinct.
	std::list<Instance *> instancesToCompare;
	bool continueUnification;
	do {

		continueUnification = false;

		instancesToCompare.clear();
		for (auto &instance : instances) {

			if (!instance.unifiedWith)
				instancesToCompare.push_back(&instance);
		}

		auto referenceIt = instancesToCompare.begin();
		auto end = instancesToCompare.end();

		while (referenceIt != end) {

			Instance *referenceInstance = *referenceIt;

			for (auto it = std::next(referenceIt); it != end; ++it) {

				if (InstancesEqual(referenceInstance, *it)) {

					UnifyInstances(referenceInstance, *it, instanceEntities);
					continueUnification = true;
				}
			}

			++referenceIt;
			while ((referenceIt != end) && (*referenceIt)->unifiedWith)
				++referenceIt;
		}

	} while (continueUnification);

	// Compute hashes for modules that have them included in their name
	for (auto &instance : instances)
		if (instance.unifiedWith == nullptr) {

			auto hashPosition = instance.userModuleName.find("%Hash%");
			if (hashPosition != std::string::npos) {

				std::size_t hash = InstanceHash(instance);

				std::stringstream ss;
				ss << std::setw(sizeof(hash) / 8) << std::setfill('0') << std::hex << hash;
				instance.userModuleName.replace(hashPosition, 6, ss.str());
			}
		}

	// Resolve ! character at the beginning of a module name. These are so-called models may or may not take part in code generation
	for (auto &instance : instances)
		if (instance.unifiedWith == nullptr) {

			if (instance.userModuleName[0] == '!') {

				instance.userModuleName.erase(0, 1);
				instance.isModel = true;
			}
		}


	// Assign proper module names and check for name clashes
	std::map<std::string, std::list<Instance *>> nameAssignments;
	for (auto &instance : instances)
		if (instance.unifiedWith == nullptr)
			nameAssignments[instance.userModuleName].push_back(&instance);

	for (auto &assignment : nameAssignments) {

		if (assignment.second.size() == 1) {

			if (!assignment.first.empty())
				assignment.second.front()->moduleName = assignment.first;
			else 
				assignment.second.front()->moduleName = assignment.second.front()->GetFullName();
		}
		else {

			for (auto *instance : assignment.second) {

				instance->moduleName = instance->GetFullName();
				std::cout << "Name clash: '" << assignment.first << "' <-- '" << instance->moduleName << "'." << std::endl;
			}
		}
	}

	// Replace invalid characters by underscores.
	for (auto &instance : instances)
		std::transform(instance.moduleName.begin(), instance.moduleName.end(), instance.moduleName.begin(), [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ? c : '_'; });


	// Print summary
	int longestModuleName = std::accumulate(instances.begin(), instances.end(), 10, [](int current, Instance const &instance) { return std::max(current, (int)instance.moduleName.length()); });

	os << std::endl;
	os << "  instance" << std::string(longestModuleName - 8, ' ') << " | " << std::setw(10) << "inputs" << " | " << std::setw(10) << "outputs" << " | " << std::setw(10) << "entities" << std::endl;
	os << " " << std::string(longestModuleName + 1, '-') << "-+-" << std::string(10, '-') << "-+-" << std::string(10, '-') << "-+-" << std::string(10, '-') << "-" << std::endl;

	for (auto &instance : instances) {

		if (instance.unifiedWith)
			continue;

		os << "  " << instance.moduleName << std::string(longestModuleName - instance.moduleName.length(), ' ') << " | ";
		os << std::setw(10) << instance.inputPorts.size() << " | ";
		os << std::setw(10) << instance.outputPorts.size() << " | ";
		os << std::setw(10) << instance.entities.size() << std::endl;
	}
	os << std::endl;
}

}
}
