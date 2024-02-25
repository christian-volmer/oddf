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

#include "global.h"
#include "debug.h"

namespace dfx {
namespace debug {

modules::Logger Logger;

std::string internalLastBlockName;
std::string internalCurrentHierarchyName;

std::string const &lastBlockName = internalLastBlockName;
std::string const &currentHierarchyName = internalCurrentHierarchyName;

void Warning(std::string const &message)
{
	std::cout << "WARNING: block '" + dfx::Design::GetCurrent().GetHierarchy().GetCurrentLevel()->GetFullName() + "': " + message + "\n";
}





}
}
