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

#include <oddf/Simulator.h>

#include "simulation/internal/SimulatorImpl.h"

namespace oddf {

Simulator::Simulator() :
	m_impl(new simulation::internal::SimulatorImpl)
{
}

Simulator::~Simulator()
{
}

bool Simulator::RegisterSimulatorBlockFactory(design::backend::DesignBlockClass const &designBlockClass,
	std::unique_ptr<simulation::backend::SimulatorBlockFactoryBase> &&simulatorBlockFactory)
{
	return m_impl->RegisterSimulatorBlockFactory(designBlockClass, std::move(simulatorBlockFactory));
}

void Simulator::TranslateDesign(design::backend::IDesign const &design)
{
	m_impl->TranslateDesign(design);
}

} // namespace oddf
