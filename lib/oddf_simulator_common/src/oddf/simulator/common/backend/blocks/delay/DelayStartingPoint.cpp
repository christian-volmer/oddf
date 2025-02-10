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

#include "../Delay.h"

#include "../../instructions/Copy.h"
#include "DelayObject.h"

namespace oddf::simulator::common::backend::blocks {

DelayStartingPoint::DelayStartingPoint(design::blocks::backend::IDesignBlock const *originalDesignBlock, design::NodeType const &type, DelayEndpoint const &endpoint) :
	SimulatorBlockBase(0, { type }),
	m_originalDesignBlock(originalDesignBlock),
	m_endpoint(endpoint),
	m_pState()
{
}

std::string DelayStartingPoint::GetDesignPathHint() const
{
	return m_originalDesignBlock->GetPath() + ":StartingPoint";
}

void DelayStartingPoint::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	auto &delayObject = context.GetOrConstructComponentObject<DelayObject>(context.GetCurrentComponent());

	m_pState = delayObject.AddState<types::Boolean>();

	context.EmitInstruction<instructions::Copy<types::Boolean>>(m_pState->m_value);
}

void DelayStartingPoint::Finalise(ISimulatorFinalisationContext &)
{
	m_pState->m_pSource = m_endpoint.GetInputsList()[0].GetDriver().GetPointer<types::Boolean>();
}

} // namespace oddf::simulator::common::backend::blocks
