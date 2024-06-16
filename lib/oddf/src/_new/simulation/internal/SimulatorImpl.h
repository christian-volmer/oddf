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

#include <oddf/design/backend/IDesign.h>
#include <oddf/design/backend/IDesignBlock.h>
#include <oddf/simulation/backend/SimulatorBlockFactoryBase.h>

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace oddf {
namespace simulation {

namespace backend {

class SimulatorBlockBase;

} // namespace backend

namespace internal {

class SimulatorImpl {

private:

	std::vector<std::unique_ptr<simulation::backend::SimulatorBlockBase>> m_blocks;

	std::map<design::backend::DesignBlockClass, std::unique_ptr<simulation::backend::SimulatorBlockFactoryBase>> m_simulatorBlockFactories;

	void RegisterDefaultBlockFactories();

public:

	SimulatorImpl();
	~SimulatorImpl();

	bool RegisterSimulatorBlockFactory(design::backend::DesignBlockClass const &designBlockClass,
		std::unique_ptr<simulation::backend::SimulatorBlockFactoryBase> &&simulatorBlockFactory);

	void TranslateDesign(design::backend::IDesign const &design);
};

} // namespace internal
} // namespace simulation
} // namespace oddf
