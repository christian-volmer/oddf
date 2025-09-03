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

    Declares class `SimulatorBlockOutput::SimulatorNode`.

*/

#pragma once

#include <oddf/simulator/common/backend/SimulatorBlockOutput.h>

#include <oddf/simulator/common/backend/ISimulatorComponent.h>
#include <oddf/design/NodeType.h>

namespace oddf::simulator::common::backend {

/*
    Implementation of `ISimulatorNode`, which becomes returned by member
    function `CreateSimulatorNode()`
*/
class SimulatorBlockOutput::SimulatorNode : public virtual simulator::backend::ISimulatorNode {

private:

	class SimulatorNodeAccessBool;
	class SimulatorNodeAccessFixedPoint;

	ISimulatorComponent *m_component;
	design::NodeType m_type;
	void const *m_pointer;

public:

	SimulatorNode(ISimulatorComponent *component, design::NodeType type, void const *pointer);

	~SimulatorNode() = default;

	SimulatorNode(SimulatorNode const &) = delete;
	void operator=(SimulatorNode const &) = delete;

	//
	// ISimulatorNode members
	//

	virtual design::NodeType GetType() const override;
	virtual std::unique_ptr<simulator::backend::ISimulatorNodeAccess> GetAccess() const override;

	//
	// IObject members
	//

	virtual void *GetInterface(oddf::Uid const &iid) override;
};

} // namespace oddf::simulator::common::backend
