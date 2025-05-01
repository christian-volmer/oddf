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

#include <oddf/ResourcePath.h>
#include <oddf/Exception.h>

#include <algorithm>
#include <utility>

namespace oddf {

ResourcePath ResourcePath::Parse(std::string str)
{
	ResourcePath result;
	result.AssignFromString(str);
	return result;
}

void ResourcePath::AssignFromString(std::string str)
{
	m_elements.clear();

	if (str == ".") {

		m_isAbsolute = false;
		return;
	}

	if (str == "/") {

		m_isAbsolute = true;
		return;
	}

	if (str.empty())
		throw Exception(ExceptionCode::InvalidArgument, "Parameter `str` must not be empty.");

	m_isAbsolute = str[0] == '/';
	size_t current = str[0] == '/' ? 1 : 0;
	size_t next;

	m_elements.clear();

	while ((next = str.find('/', current)) != str.npos) {

		auto element = str.substr(current, next - current);

		if (element.empty())
			throw Exception(ExceptionCode::InvalidArgument, "Parameter `str` must not contain double slashes.");

		if (!IsValidElement(element))
			throw Exception(ExceptionCode::InvalidArgument, "Parameter `str` contains characters that are not allowed as part of a resource path.");

		m_elements.push_back(element);
		current = next + 1;
	}

	auto element = str.substr(current, next - current);

	if (element.empty())
		throw Exception(ExceptionCode::InvalidArgument, "Parameter `str` must not end in a slash.");

	if (!IsValidElement(element))
		throw Exception(ExceptionCode::InvalidArgument, "Parameter `str` contains characters that are not allowed as part of a resource path.");

	m_elements.push_back(element);
}

std::string ResourcePath::ToString() const
{
	if (m_elements.empty())
		return m_isAbsolute ? "/" : ".";

	std::string result;
	bool first = true;

	for (auto const &element : m_elements) {

		if (m_isAbsolute || !first)
			result += "/";
		result += element;
		first = false;
	}

	return result;
}

ResourcePath ResourcePath::Parent() const
{
	ResourcePath parent = *this;

	if (parent.m_elements.empty())
		throw Exception(ExceptionCode::IllegalMethodCall, "The resource path has no parent because it is empty.");

	parent.m_elements.pop_back();

	return parent;
}

ResourcePath ResourcePath::Append(ResourcePath const &other) const
{
	if (other.m_isAbsolute)
		return other;
	else {

		ResourcePath result = *this;

		result.m_elements.insert(result.m_elements.end(), other.m_elements.begin(), other.m_elements.end());
		return result;
	}
}

bool ResourcePath::IsValidElement(std::string const &str)
{
	if (str.empty())
		return false;

	return std::all_of(str.begin(), str.end(), ResourcePath::IsValidCharacter);
}

} // namespace oddf
