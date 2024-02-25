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

	An 'enabled scope' is a portion of the design, where the 'enable'
	input of flip-flops is connected to a common design node.

*/

#pragma once

namespace dfx {


//
// EnabledScope class
//

class EnabledScope {

private:

	bool previousHasEnable;
	node<bool> previousEnable;

public:

	EnabledScope(node<bool> const &enableSignal)
	{
		auto &currentDesign = Design::GetCurrent();

		previousEnable = currentDesign.customDefaultEnable;
		previousHasEnable = currentDesign.hasCustomDefaultEnable;

		if (previousHasEnable) {

			currentDesign.customDefaultEnable = previousEnable && enableSignal;
		}
		else {

			currentDesign.hasCustomDefaultEnable = true;
			currentDesign.customDefaultEnable = enableSignal;
		}
	}

	EnabledScope(EnabledScope const &) = delete;

	~EnabledScope()
	{
		Design::GetCurrent().hasCustomDefaultEnable = previousHasEnable;
		Design::GetCurrent().customDefaultEnable = previousEnable;
	}

	void operator=(EnabledScope const &) = delete;
};

#define DFX_ENABLED_SCOPE(enableSignal) dfx::EnabledScope _enabledScope(enableSignal)

}
