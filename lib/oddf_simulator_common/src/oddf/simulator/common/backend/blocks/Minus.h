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

    Simulator support for the 'negate' design block.

*/

#pragma once

#include <oddf/simulator/common/backend/SimulatorBlockBase.h>

namespace oddf::simulator::common::backend::blocks {

//
// NotMaster
//

class Minus : public SimulatorBlockBase {

public:

	Minus(design::blocks::backend::IDesignBlock const &designBlock);

	virtual std::string GetDesignPathHint() const override;

	virtual void Elaborate(ISimulatorElaborationContext &context) override;

	virtual void GenerateCode(ISimulatorCodeGenerationContext &context) override;
};

} // namespace oddf::simulator::common::backend::blocks
