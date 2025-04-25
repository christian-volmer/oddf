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

    Provides the `ResourcePath` class, which specifies hierarchical paths
    to resources within the ODDF system.

*/

#pragma once

#include <string>
#include <vector>

namespace oddf {

/*
    Specifies hierarchical paths to resources within the ODDF system.
*/
class ResourcePath {

private:

	// An absolute resource path starts with a /, a relative one does not.
	bool m_isAbsolute;
	std::vector<std::string> m_elements;

public:

	ResourcePath(std::string str);
	ResourcePath(ResourcePath const &) = default;

	ResourcePath &operator=(std::string str);
	ResourcePath &operator=(ResourcePath const &) = default;

	// Returns the resource path as a string.
	std::string ToString() const;

	// Returns a resource path containing all elements but the last one.
	ResourcePath Parent() const;

	// If `other` is an absolute resource path it will replace the current one. If it
	// is relative it will append its elements to the current one. The function returns
	// a reference to the current, modified resource path.
	ResourcePath &Append(ResourcePath const &other);

	bool IsAbsolute() const noexcept
	{
		return m_isAbsolute;
	}

	bool IsRelative() const noexcept
	{
		return !m_isAbsolute;
	}

	static constexpr bool IsValidPathCharacter(char c)
	{
		return (c == '$') || (c == '_') || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}
};

} // namespace oddf
