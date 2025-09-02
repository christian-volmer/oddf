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

#include "../SimulatorCore.h"

#include "SimulatorNodeComparer.h"
#include "SimulatorNodeHierarchyNodeComparer.h"

#include <oddf/IEnumerator.h>
#include <oddf/IHierarchyNode.h>

namespace oddf::simulator::common::backend {

struct SimulatorCore::SimulatorNodeHierarchyNode : public virtual IHierarchyNode {

	std::string m_name;
	SimulatorNodeHierarchyNode *m_parent;
	std::set<std::unique_ptr<SimulatorNodeHierarchyNode>, SimulatorNodeHierarchyNodeComparer> m_children;

	std::set<SimulatorNode, SimulatorNodeComparer> m_nodes;

	SimulatorNodeHierarchyNode(std::string const &name, SimulatorNodeHierarchyNode *parent);

	SimulatorNodeHierarchyNode(SimulatorNodeHierarchyNode const &) = delete;
	void operator=(SimulatorNodeHierarchyNode const &) = delete;

	//
	// IHierarchyNode members
	//

	virtual std::string GetName() const override;

	virtual bool HasData() const noexcept override;
	virtual std::unique_ptr<IObject> GetData() const override;

	virtual bool HasChildren() const noexcept override;
	virtual std::unique_ptr<IEnumerator<IHierarchyNode const &>> GetChildren() const override;

	virtual IHierarchyNode const *GetParent() const override;

	//
	// IObject member
	//

	virtual void *GetInterface(oddf::Uid const &iid) override;
};

} // namespace oddf::simulator::common::backend
