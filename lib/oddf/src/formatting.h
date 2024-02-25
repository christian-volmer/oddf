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

	Support for printf-style formatting into std::string.

*/

#pragma once

#include <string>

std::string internal_string_printf(char const *format, ...);

template<typename... argTs>
inline std::string string_printf(std::string const &format, argTs &&...args)
{
	char const *fmt = format.c_str();
	return internal_string_printf(fmt, std::forward<argTs>(args)...);
}
