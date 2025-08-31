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

    Provides a standard implementation of the `IReferenceCountSentinel`
    interface.

*/

#include <oddf/utility/StandardReferenceCountSentinel.h>
#include <oddf/Exception.h>

namespace oddf::utility {

StandardReferenceCountSentinel::StandardReferenceCountSentinel() :
	m_ReferenceCount(0)
{
}

StandardReferenceCountSentinel::~StandardReferenceCountSentinel()
{
	if (m_ReferenceCount)
		throw Exception(ExceptionCode::Unexpected, "An object is destroyed despite it being referenced by other objects.");
}

void StandardReferenceCountSentinel::Increment() noexcept
{
	++m_ReferenceCount;
}

void StandardReferenceCountSentinel::Decrement()
{
	if (m_ReferenceCount == 0)
		throw Exception(ExceptionCode::IllegalMethodCall, "Trying to decrease a reference count below zero.");

	--m_ReferenceCount;
}

size_t StandardReferenceCountSentinel::GetValue() const noexcept
{
	return m_ReferenceCount;
}

} // namespace oddf::utility
