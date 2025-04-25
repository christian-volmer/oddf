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

#include <cassert>

namespace oddf::simulator::common::backend {

void SimulatorCore::TranslateDesign(design::IDesign const &design)
{
	{
		auto blockMapping = MapBlocks(design);

		MapConnections(*blockMapping);
	}

	ElaborateBlocks();

	BuildComponents();

	GenerateCode();

	FinaliseTranslation();

	/*

	        Probe
	            - SimulatorObject mit Pfad "/xxx/yyy/probes:UserId".
	            - Kennt Adresse

	        Logging
	        Forcing
	        Assertions / Evaluations
	            --> C++ functions that execute on the values of simulation nodes (Leon equalise bug evaluation)
	            --> Logging could also apply functions to nodes before logging the result
	            --> Nodes must be named in the design (inputs, outputs, labels)
	        Memory
	        Sink
	        Source
	        Signal
	        Random

	*/
}

} // namespace oddf::simulator::common::backend
