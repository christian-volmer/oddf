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

#include "ISimulatorBlockMapping.h"
#include "SimulatorComponent.h"

#include <oddf/simulator/common/backend/ISimulatorBlockFactory.h>

#include <oddf/simulator/backend/ISimulatorAccess.h>

#include <oddf/design/IDesign.h>
#include <oddf/design/blocks/backend/DesignBlockClass.h>

#include <map>
#include <memory>
#include <vector>
#include <set>

namespace oddf::simulator::common::backend {

class SimulatorCore : public simulator::backend::ISimulatorAccess {

private:

	// All simulator blocks
	std::vector<std::unique_ptr<SimulatorBlockBase>> m_blocks;

	// Map of all registered simulator block factories. Simulator block factories create simulator blocks from design blocks.
	std::map<design::blocks::backend::DesignBlockClass, std::unique_ptr<ISimulatorBlockFactory>> m_simulatorBlockFactories;

	// List of all simulator components. Components are independent lists of simulator operations that can execute in parallel.
	std::list<SimulatorComponent> m_components;

	// Set of all invalid simulator components
	std::set<SimulatorComponent *> m_invalidComponents;

	std::map<std::string, std::unique_ptr<IObject>> m_namedSimulatorObjects;

	std::set<simulator::backend::IClockable *> m_clockables;

	struct SimulatorNode;
	struct SimulatorNodeComparer;
	struct SimulatorNodeHierarchyNode;
	struct SimulatorNodeHierarchyNodeComparer;

	std::unique_ptr<SimulatorNodeHierarchyNode> m_nodeHierarchyRoot;

	// Registers simulator block factories for all standard ODDF design blocks.
	void RegisterDefaultBlockFactories();

	// Creates simulator blocks based on the blocks of the given design.
	std::unique_ptr<ISimulatorBlockMapping const> MapBlocks(design::IDesign const &design);

	// Maps the connectivity from the design to the simulator.
	void MapConnections(ISimulatorBlockMapping const &blockMapping);

	// Elaborates blocks by allowing existing blocks to add new blocks to the simulator.
	void ElaborateBlocks();

	// Builds the simulator components.
	void BuildComponents();

	// Generates the final simulation code on every component.
	void GenerateCode();

	// Last step of design translation. After this the simulator will be ready.
	void FinaliseTranslation();

public:

	SimulatorCore();
	~SimulatorCore();

	SimulatorCore(SimulatorCore const &) = delete;
	void operator=(SimulatorCore const &) = delete;

	bool RegisterSimulatorBlockFactory(design::blocks::backend::DesignBlockClass const &designBlockClass,
		std::unique_ptr<ISimulatorBlockFactory> &&simulatorBlockFactory);

	void RegisterGlobalObject(std::string name, std::unique_ptr<IObject> &&object);

	void RegisterNamedNode(ResourcePath const &path, SimulatorBlockOutput const &output);

	// Translates the given design so it can be simulated by this simulator instance.
	void TranslateDesign(design::IDesign const &design);

	void Run(size_t cycles);

	// Marks the state of the specified component invalid.
	void InvalidateComponentState(SimulatorComponent &component);

	// Ensures the state of the specified component is valid by executing the component code if necessary.
	void EnsureValidComponentState(SimulatorComponent &component);

	// Ensures the states of all components are valid by executing component code as necessary.
	void EnsureAllComponentStatesValid();

	//
	// simulator::backend::ISimulatorAccess members
	//

	virtual void *GetNamedObjectInterface(std::string const &name, Uid const &iid) const override;

	virtual void RegisterClockable(simulator::backend::IClockable &clockable) override;
	virtual void UnregisterClockable(simulator::backend::IClockable &clockable) override;

	virtual IHierarchyNode const &GetNodeHierarchyRoot() const override;
};

} // namespace oddf::simulator::common::backend
