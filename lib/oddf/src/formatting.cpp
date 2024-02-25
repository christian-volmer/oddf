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

#include "formatting.h"

#include <cstdarg>
#include <cstdio>
#include <memory>

std::string internal_string_printf(char const *format, ...)
{
	std::va_list args;

	// Compute resulting string size including the null-terminator.
    va_start(args, format);
	size_t length = std::vsnprintf(nullptr, 0, format, args) + 1;
	va_end(args);

	std::unique_ptr<char[]> buffer(new char[length]);

    va_start(args, format);
	std::vsnprintf(buffer.get(), length, format, args);
	va_end(args);

	return std::string(buffer.get(), buffer.get() + length - 1);
}
