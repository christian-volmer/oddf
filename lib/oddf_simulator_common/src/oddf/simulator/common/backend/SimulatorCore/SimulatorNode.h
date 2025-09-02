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

    Declaration of class `SimulatorCore::SimulatorNode`, which implements
    the `simulator::backend::ISimulatorNode` interface.

*/

#pragma once

#include "../SimulatorCore.h"

#include <oddf/simulator/backend/ISimulatorNode.h>

namespace oddf::simulator::common::backend {

/*
    Implementation of the `simulator::backend::ISimulatorNode` interface
*/
struct SimulatorCore::SimulatorNode : public virtual simulator::backend::ISimulatorNode {

private:

	std::string m_name;
	design::NodeType m_type;
	void const *m_pointer;

	friend struct SimulatorCore::SimulatorNodeComparer;

public:

	SimulatorNode(std::string const &name, SimulatorBlockOutput const *output);

	SimulatorNode(SimulatorNode const &) = delete;
	void operator=(SimulatorNode const &) = delete;

	SimulatorNode(SimulatorNode &&) = default;
	SimulatorNode &operator=(SimulatorNode &&) = default;

	//
	// ISimulatorNode members
	//

	virtual std::string GetName() const override;
	virtual design::NodeType GetType() const override;
	virtual std::unique_ptr<simulator::backend::ISimulatorNodeAccess> GetAccess() const override;

	//
	// IObject member
	//

	virtual void *GetInterface(oddf::Uid const &iid) override;
};

} // namespace oddf::simulator::common::backend
