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

#include "SimulatorBlockBase.h"
#include "SimulatorBlockFactoryBase.h"

#include <memory>

namespace oddf {
namespace simulation {
namespace backend {

template<typename blockFactoryT>
class GenericSimulatorBlockFactory : public SimulatorBlockFactoryBase {

public:

	virtual std::unique_ptr<SimulatorBlockBase> CreateFromDesignBlock(design::backend::IDesignBlock const &designBlock) const override
	{
		return std::make_unique<blockFactoryT>(designBlock);
	}
};

} // namespace backend
} // namespace simulation
} // namespace oddf
