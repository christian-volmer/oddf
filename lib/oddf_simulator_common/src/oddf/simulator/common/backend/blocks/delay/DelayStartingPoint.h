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

#include "DelayEndpoint.h"
#include "DelayElement.h"

namespace oddf::simulator::common::backend::blocks {

class DelayStartingPoint : public SimulatorBlockBase {

private:

	DelayEndpoint const &m_endpoint;
	DelayElement *m_delayElement;

public:

	DelayStartingPoint(design::blocks::backend::IDesignBlock const *originalDesignBlock,
		design::NodeType const &type, DelayEndpoint const &endpoint);

	DelayStartingPoint(DelayStartingPoint const &) = delete;
	void operator=(DelayStartingPoint const &) = delete;

	virtual std::string GetDesignPathHint() const override;

	virtual void Elaborate(ISimulatorElaborationContext &) override { }
	virtual void GenerateCode(ISimulatorCodeGenerationContext &context) override;
	virtual void Finalise(ISimulatorFinalisationContext &) override;
};

} // namespace oddf::simulator::common::backend::blocks
