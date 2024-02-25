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

	Function to combine hash values in a non-trivial way.

*/

#pragma once

#include <functional>

/*

The following function hash_combine() is 

	Copyright 2005-2008 Daniel James
	Distributed under the Boost Software License, Version 1.0
	(See accompanying file /misc/LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

Notes:

	- More information at https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine

	- TODO: this might not be the best way to combine hashes, see the dicsussion 
	  https://stackoverflow.com/questions/35985960/c-why-is-boosthash-combine-the-best-way-to-combine-hash-values

	- TODO: hashing gives different results on 32 and 64 bit compiles. Should be fixed.

*/

template<class T> static inline void hash_combine(std::size_t &seed, T const &v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
