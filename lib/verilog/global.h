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

	Includes frequently used header files. Choose this for precomiled
	headers if your compiler supports it.

*/

#include <cassert>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <filesystem>

#include "../oddf/src/dfx.h"
#include "../oddf/src/generator/generator.h"

namespace fs = std::filesystem; //tr2::sys;
