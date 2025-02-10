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

#include "../SimulatorCore.h"

#include "../GenericSimulatorBlockFactory.h"

#include "../blocks/Constant.h"
#include "../blocks/Delay.h"
#include "../blocks/Not.h"
#include "../blocks/Probe.h"
#include "../blocks/Signal.h"
#include "../blocks/Temp.h"

namespace oddf::simulator::common::backend {

void SimulatorCore::RegisterDefaultBlockFactories()
{
	using design::blocks::backend::DesignBlockClass;

	RegisterSimulatorBlockFactory(DesignBlockClass("constant"), MakeSimulatorBlockFactory<blocks::ConstantMaster>());
	RegisterSimulatorBlockFactory(DesignBlockClass("delay"), MakeSimulatorBlockFactory<blocks::DelayMaster>());
	RegisterSimulatorBlockFactory(DesignBlockClass("not"), MakeSimulatorBlockFactory<blocks::NotMaster>());
	RegisterSimulatorBlockFactory(DesignBlockClass("probe"), MakeSimulatorBlockFactory<blocks::Probe>());
	RegisterSimulatorBlockFactory(DesignBlockClass("signal"), MakeSimulatorBlockFactory<blocks::Signal>());
	RegisterSimulatorBlockFactory(DesignBlockClass("temporary"), MakeSimulatorBlockFactory<blocks::TempMaster>());
}

} // namespace oddf::simulator::common::backend
