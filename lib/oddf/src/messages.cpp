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

	Informational and error messages.

*/

#include "global.h"

#include "messages.h"

namespace dfx {

design_error::design_error(char const *what_arg) : logic_error(std::string("ERROR: ") + std::string(what_arg))
{
	std::cout << what() << std::endl;
}


design_error::design_error(std::string const &what_arg) : logic_error(std::string("ERROR: ") + std::string(what_arg))
{
	std::cout << what() << std::endl;
}

void design_info(std::string const &info)
{
	std::cout << "INFO: " << info << std::endl;
}


}
