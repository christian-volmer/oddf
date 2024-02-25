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

#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <list>
#include <vector>
#include <map>

#include "../node.h"
#include "properties.h"

namespace dfx {
namespace generator {

class Instance;
class Entity;

class Entity {

public:

	struct Output;

	struct Input {

		Entity *entity;
		int index;
		std::string name;
		Output *driver;
	};

	struct Output {

	private:

		int index;
		std::string name;
		std::list<Input *> targets;

		friend class Entity;
		friend class Generator;
		friend class Instance;

	public:

		Output() : 
			index(0), 
			name(), 
			targets(), 
			entity(nullptr), 
			type(), 
			groupIndex(0),
			busSize(-1), 
			busIndex(0)
		{
		}

		std::string GetBaseName() const
		{
			return name;
		}

		void SetBaseName(std::string const &newName)
		{
			name = newName;
		}

		Entity *entity;
		types::TypeDescription type;
		int groupIndex;
		int busSize; // -1 signifies a non-bus node.
		int busIndex;
	};

	dfx::backend::BlockBase const *block;

	int index;

	std::string className;
	std::string name;
	Instance *instance;

	std::vector<Input> inputs;
	std::vector<Output> outputs;

	Properties properties;

	// Used by entities of class 'instance' and points to the instantiated module.
	Instance *targetModule;

	Entity(Instance *instance, dfx::backend::BlockBase const *block);
	Entity(Instance *instance, std::string const &className, std::string const &name, std::vector<std::string> const &inputNames, std::vector<std::string> const &outputNames);
	Entity(Instance *instance, std::string const &className, std::string const &name, int numberOfInputs, int numberOfOutputs);

	bool IsConsistent() const;
};


class Instance {

public:

	Instance *parent;
	std::list<Entity> entities;

	// Sequence number of the instance
	int sequenceNumber;

	// Name of the instance
	std::string name;

	// Name given to the inferred module
	std::string moduleName;

	// Name given to the module by the user
	std::string userModuleName;

	// Models may or may not take part in code generation.
	bool isModel;

	// For module generation. This instance is identical to this one
	Instance *unifiedWith;

	// References to 'input_port' entities
	std::vector<Entity *> inputPorts;

	// References to 'output_port' entities
	std::vector<Entity *> outputPorts;

	bool ChildOf(Instance const *rhs) const;
	std::string GetFullName() const;

	std::size_t GetHash() const;

	Instance(int sequenceNumber, std::string const &name, std::string const &moduleName, Instance *parent) :
		parent(parent),
		entities(),
		sequenceNumber(sequenceNumber),
		name(name),
		moduleName(),
		userModuleName(moduleName),
		isModel(false),
		unifiedWith(nullptr),
		inputPorts(),
		outputPorts(),
		cachedHashValid(false),
		cachedHash(0)
	{
	}

private:

	mutable bool cachedHashValid;
	mutable std::size_t cachedHash;
};


class Generator {

private:

	//
	// generator.cpp
	//

	std::unordered_map<dfx::HierarchyLevel const *, Instance *> instance_mapping;
	std::unordered_map<dfx::backend::BlockBase const *, Entity *> entity_mapping;

	Instance *ToInstance(dfx::HierarchyLevel const *hierarchyLevel);
	static void MakeConnection(Entity::Output &output, Entity::Input &input);
	static void BreakConnection(Entity::Input &input);

	void MapEntities(dfx::Design const &design, std::basic_ostream<char> &os);
	void MapConnections(std::basic_ostream<char> &os);
	void ReportUnconnectedOutputs(std::basic_ostream<char> &os);

	void CheckConsistency(std::basic_ostream<char> &os);

	//
	// generator_port_placing.cpp
	//
	
	bool PlaceInputPorts(bool firstPass);
	bool PlaceInputConnectors();
	bool PlaceOutputPorts(bool firstPass);
	bool PlaceOutputConnectors();
	void PlacePorts(std::basic_ostream<char> &os);

	//
	// generator_port_naming.cpp
	//

	void AssignBusInputLabels(Instance &instance, std::unordered_set<Entity *> &unassignedInputPorts, std::list<Entity *> &inputLabels, bool disableBussification);
	void AssignNodeInputLabels(Instance &instance, std::unordered_set<Entity *> &unassignedInputPorts, std::list<Entity *> &inputLabels);
	void AssignBusOutputLabels(Instance &instance, std::unordered_set<Entity *> &unassignedOutputPorts, std::list<Entity *> &outputLabels, bool disableBussification);
	void AssignNodeOutputLabels(Instance &instance, std::unordered_set<Entity *> &unassignedOutputPorts, std::list<Entity *> &outputLabels);
	void NamePorts(std::basic_ostream<char> &os);

	//
	// generator_instances.cpp
	//

	void IdentifyInstances(std::basic_ostream<char> &os);

	static bool InstancesEqual(Instance const *instance1, Instance const *instance2);
	static std::size_t InstanceHash(Instance const &instance);
	static void UnifyInstances(Instance *referenceInstance, Instance *instance, std::list<Entity *> &instanceEntities);

	void GenerateModules(std::basic_ostream<char> &os);

public:

	struct Configuration {

		// Enable bussification globally.
		bool globalBussification;

		// Enable or disable bussification per module.
		std::map<std::string, bool> moduleBussification;

		Configuration();
	};

private:

	Configuration configuration;

public:

	std::list<Instance> instances;
	Generator(Design const &design, std::basic_ostream<char> &os, Configuration const &theConfiguration = Configuration());
};

}
}
