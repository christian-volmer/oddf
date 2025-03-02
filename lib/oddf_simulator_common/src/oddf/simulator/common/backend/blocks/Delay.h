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

    Simulator support for the 'delay' design block.

*/

#pragma once

#include <oddf/simulator/common/backend/SimulatorBlockBase.h>

#include "delay/DelayObject.h"
#include "../instructions/Copy.h"

namespace oddf::simulator::common::backend::blocks {

//
// DelayMaster
//

class DelayMaster : public SimulatorBlockBase {

public:

	DelayMaster(design::blocks::backend::IDesignBlock const &designBlock);

	virtual std::string GetDesignPathHint() const override;

	virtual void Elaborate(ISimulatorElaborationContext &context) override;
};

//
// DelayEndpoint
//

class DelayEndpoint : public SimulatorBlockBase {

private:

	design::blocks::backend::IDesignBlock const *m_originalDesignBlock;

public:

	DelayEndpoint(design::blocks::backend::IDesignBlock const *originalDesignBlock);

	DelayEndpoint(DelayEndpoint const &) = delete;
	void operator=(DelayEndpoint const &) = delete;

	virtual std::string GetDesignPathHint() const override;

	virtual void Elaborate(ISimulatorElaborationContext &) override { }
	virtual void GenerateCode(ISimulatorCodeGenerationContext &) override { }
};

//
// DelayStartingPoint
//

template<typename T>
class DelayStartingPoint : public SimulatorBlockBase {

private:

	design::blocks::backend::IDesignBlock const *m_originalDesignBlock;
	design::NodeType m_type;
	DelayEndpoint const &m_endpoint;
	DelayState<T> *m_pState;

public:

	DelayStartingPoint(design::blocks::backend::IDesignBlock const *originalDesignBlock, design::NodeType const &type, DelayEndpoint const &endpoint) :
		SimulatorBlockBase(0, { type }),
		m_originalDesignBlock(originalDesignBlock),
		m_type(type),
		m_endpoint(endpoint),
		m_pState()
	{
	}

	DelayStartingPoint(DelayStartingPoint<T> const &) = delete;
	void operator=(DelayStartingPoint<T> const &) = delete;

	virtual std::string GetDesignPathHint() const override
	{
		return m_originalDesignBlock->GetPath() + ":StartingPoint";
	}

	virtual void Elaborate(ISimulatorElaborationContext &) override { }

	virtual void GenerateCode(ISimulatorCodeGenerationContext &context) override
	{
		auto &delayObject = context.GetOrConstructComponentObject<DelayObject>(context.GetCurrentComponent());

		if constexpr (types::IsValueType<T>) {

			m_pState = delayObject.AddState<T>();
			context.EmitInstruction<instructions::Copy<T>>(m_pState->ReferenceToCurrent());
		}
		else {

			size_t elementCount = T::RequiredElementCount(m_type);
			m_pState = delayObject.AddState<T>(elementCount);

			context.EmitInstructionVariadic<instructions::Copy<T>>(
				instructions::Copy<T>::GetVariadicMember(), elementCount,
				m_pState->ReferenceToCurrent(), elementCount);
		}
	}

	virtual void Finalise(ISimulatorFinalisationContext &) override
	{
		if constexpr (types::IsValueType<T>) {

			m_pState->SetSource(m_endpoint.GetInputsList()[0].GetDriver().GetPointer<T>());
		}
		else {

			size_t elementCount = T::RequiredElementCount(m_type);
			m_pState->SetSource(m_endpoint.GetInputsList()[0].GetDriver().GetPointer<T>(elementCount));
		}
	}
};

} // namespace oddf::simulator::common::backend::blocks
