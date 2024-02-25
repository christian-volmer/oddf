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

	Preprocessor defines that control optimisations of the simulator.

*/

#pragma once

// Independent parts of the execution graph are grouped into 'components'. 
// Each component has a dirty flag that indicates whether or not a component
// needs re-evaluation.
#define DFX_SIMULATOR_ENABLE_COMPONENTS 1

// Component evaluation is done on parallel threads. Requires 
// 'DFX_SIMULATOR_ENABLE_COMPONENTS' to be 1.
#define DFX_SIMULATOR_USE_MULTITHREADING 1

// 'Delay' element marks dependent components as dirty only
// if the delay output actually changed value. Otherwise
// decision is based solely on the state of the 'enable' input.
// Requires 'DFX_SIMULATOR_ENABLE_COMPONENTS' to be 1.
#define DFX_SIMULATOR_DELAY_DIRTY_ON_CHANGE 1
