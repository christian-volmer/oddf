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

	Classes and functions that support the debugging of a design.

*/

#pragma once

#include "modules/logger.h"
#include "blocks/function.h"

namespace dfx {
namespace debug {

extern modules::Logger Logger;

extern std::string const &lastBlockName;
extern std::string const &currentHierarchyName;

inline void Assert(node<bool> const &condition, std::string const &message)
{
	dfx::blocks::Function(condition, [=](bool cond) {

		if (cond == false)
			throw design_error("Runtime assertion failed: " + message);

		return false;
	});
}

void Warning(std::string const &message);



}
}
