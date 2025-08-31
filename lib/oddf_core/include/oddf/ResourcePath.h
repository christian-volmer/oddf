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

	void AssignFromString(std::string str);

public:

	ResourcePath() :
		m_isAbsolute(), m_elements() { }

	ResourcePath(ResourcePath const &) = default;
	ResourcePath &operator=(ResourcePath const &) = default;

	static ResourcePath Parse(std::string str);

	// Returns the resource path as a string.
	std::string ToString() const;

	// Returns a resource path containing all elements but the last one.
	ResourcePath Parent() const;

	// Appends `other` to the current resource path and returns the result.
	// If `other` is absolute, the function just returns other.
	ResourcePath Append(ResourcePath const &other) const;

	bool IsAbsolute() const noexcept
	{
		return m_isAbsolute;
	}

	bool IsRelative() const noexcept
	{
		return !m_isAbsolute;
	}

	static constexpr bool IsValidCharacter(char c)
	{
		// All printable characters are allowed except space, delete, slash, and backslash
		return (c > 32) && (c < 127) && (c != '/') && (c != '\\');
	}

	static bool IsValidElement(std::string const &str);

	auto begin() const
	{
		return m_elements.cbegin();
	}

	auto end() const
	{
		return m_elements.cend();
	}
};

} // namespace oddf
