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

#include "../SimulatorCore.h"
#include "../SimulatorBlockInternals.h"

#include <oddf/Exception.h>

#include <list>
#include <memory>
#include <cassert>
#include <iostream>
#include <set>
#include <random>
#include <algorithm>

namespace oddf::simulator::common::backend {

void SimulatorCore::BuildComponents()
{
	/*
	    Methods of class `ComponentBuilder` build the components of the simulator
	    execution graph by traversing the simulator blocks according to their
	    connectivity.
	*/
	class ComponentBuilder {

	private:

		// Reference to the simulator core.
		SimulatorCore &m_simulatorCore;

		// Reference to the list of components of the `SimulatorCore` class instance.
		std::list<SimulatorComponent> &m_components;

		// List of pointers to released, empty components. They can be reused and speed up the memory management.
		std::list<SimulatorComponent *> m_releasedComponents;

		// Pointer to the component that is currently being built.
		SimulatorComponent *m_currentComponent;

		// Returns a pointer to a new, empty component.
		SimulatorComponent *NewComponent()
		{
			SimulatorComponent *component = nullptr;

			if (!m_releasedComponents.empty()) {

				component = m_releasedComponents.back();
				m_releasedComponents.pop_back();
			}
			else {

				component = &m_components.emplace_back(m_simulatorCore);
			}

			assert(component);
			assert(component->IsEmpty());

			return component;
		}

		// Releases an empty component so it can be reused or freed later on.
		void ReleaseComponent(SimulatorComponent *component)
		{
			assert(component);
			assert(component->IsEmpty());

			m_releasedComponents.push_back(component);
		}

		// Returns the set of blocks that have outputs that drive `block`.
		auto GetDrivingBlocks(SimulatorBlockBase &block)
		{
			std::set<SimulatorBlockBase *> drivingBlocks;

			for (auto &input : block.m_internals->m_inputs) {

				if (input.IsConnected())
					drivingBlocks.insert(&input.m_driver->m_owningBlock);
			}

			return drivingBlocks;
		}

		// Builds a component by Visiting `block` and recursively all blocks that drive it.
		SimulatorBlockBase const *VisitBlock(SimulatorBlockBase &block)
		{
			if (block.m_internals->m_component) {

				assert(!block.m_internals->m_visiting);

				SimulatorComponent *otherComponent = block.m_internals->m_component;

				if (otherComponent != m_currentComponent) {

					/*
					    Blocks from 'otherComponent' and 'm_currentComponent' must
					    be merged. Since blocks from 'otherComponent' had already
					    been visited, they need to be placed before elements from
					    'm_currentComponent'.

					    TODO

					    Confirm these cases are really visited and that they work
					    properly by activating block shuffling below.
					*/

					if (m_currentComponent->GetSize() < otherComponent->GetSize()) {

						otherComponent->MoveAppendFromOther(m_currentComponent);
						ReleaseComponent(m_currentComponent);
						m_currentComponent = otherComponent;
					}
					else {

						m_currentComponent->MovePrependFromOther(otherComponent);
						ReleaseComponent(otherComponent);
					}
				}

				return nullptr;
			}

			if (block.m_internals->m_visiting) {

				std::cout << "INFO: A loop was detected in the execution graph starting at block '" << block.GetDesignPathHint() << "'\n";
				return &block;
			}

			block.m_internals->m_visiting = true;

			auto drivingBlocks = GetDrivingBlocks(block);

			for (auto *drivingBlock : drivingBlocks) {

				auto *loop = VisitBlock(*drivingBlock);
				if (loop) {

					if (loop != &block) {

						std::cout << "INFO: ... going through block '" << block.GetDesignPathHint() << "' ...\n";
						return loop;
					}
					else {

						std::cout << "INFO: ... and returning to block '" << block.GetDesignPathHint() << "' again.\n";
						throw oddf::Exception(oddf::ExceptionCode::Fail);
					}
				}
			}

			block.m_internals->m_visiting = false;

			m_currentComponent->AddBlock(block);
			return nullptr;
		}

	public:

		ComponentBuilder(SimulatorCore &simulatorCore) :
			m_simulatorCore(simulatorCore),
			m_components(simulatorCore.m_components),
			m_releasedComponents(),
			m_currentComponent()
		{
		}

		ComponentBuilder(ComponentBuilder const &) = delete;
		void operator=(ComponentBuilder const &) = delete;

		// Build all graph components by visiting all simulator blocks.
		void BuildComponents(std::vector<std::unique_ptr<SimulatorBlockBase>> &blocks)
		{
			for (auto &block : blocks) {

				m_currentComponent = NewComponent();
				assert(block);
				VisitBlock(*block);
			}

			m_components.remove_if([](auto const &component) { return component.IsEmpty(); });
		}
	};

	/*

	    // Shuffle the blocks to debug our component generation algorithm.

	    std::random_device rd;
	    std::mt19937 g(rd());

	    std::shuffle(m_blocks.begin(), m_blocks.end(), g);

	*/

	auto componentBuilder = ComponentBuilder(*this);
	componentBuilder.BuildComponents(m_blocks);
}

} // namespace oddf::simulator::common::backend
