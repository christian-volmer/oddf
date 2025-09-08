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

#include "DelayStartingPoint.h"
#include "DelayObject.h"

#include "../Copy/CopyCode.h"

namespace oddf::simulator::common::backend::blocks {

DelayStartingPoint::DelayStartingPoint(design::blocks::backend::IDesignBlock const *originalDesignBlock,
	design::NodeType const &type, DelayEndpoint const &endpoint) :
	SimulatorBlockBase(originalDesignBlock, 0, { type }),
	m_endpoint(endpoint),
	m_delayElement()
{
}

std::string DelayStartingPoint::GetDesignPathHint() const
{
	if (m_endpoint.m_busIndex >= 0)
		return GetDesignBlockReference()->GetPath().ToString() + "<" + std::to_string(m_endpoint.m_busIndex) + ">:StartingPoint";
	else
		return GetDesignBlockReference()->GetPath().ToString() + ":StartingPoint";
}

void DelayStartingPoint::GenerateCode(ISimulatorCodeGenerationContext &context)
{
	auto const &output = GetOutputsList()->Item(0);

	auto &delayObject = context.GetOrConstructComponentObject<DelayObject>(context.GetCurrentComponent());

	m_delayElement = delayObject.AddDelayElement(output.GetType());

	switch (output.GetType().GetTypeId()) {

		case design::NodeType::BOOLEAN:
			EmitCopyCode<types::Boolean>(context, output, m_delayElement->GetStateData<types::Boolean>());
			break;

		case design::NodeType::FIXED_POINT:
			EmitCopyCode<types::FixedPoint>(context, output, m_delayElement->GetStateData<types::FixedPoint>());
			break;

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

void DelayStartingPoint::Finalise(ISimulatorFinalisationContext &)
{
	auto const &output = GetOutputsList()->Item(0);

	switch (output.GetType().GetTypeId()) {

		case design::NodeType::BOOLEAN:
			m_delayElement->SetSource(m_endpoint.GetInputsList()->Item(0).GetDriver().GetPointer<types::Boolean>());
			break;

		case design::NodeType::FIXED_POINT:
			m_delayElement->SetSource(m_endpoint.GetInputsList()->Item(0).GetDriver().GetPointer<types::FixedPoint>());
			break;

		default:
			throw Exception(ExceptionCode::NotImplemented);
	}
}

} // namespace oddf::simulator::common::backend::blocks
