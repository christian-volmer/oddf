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

    <no description>

*/

#pragma once

#include "AbstractEnumeratorImplementation.h"
#include "StdContainerElementTypeCast.h"

#include <cassert>

namespace oddf {
namespace utility {
namespace backend {

template<typename referenceT, typename iteratorT>
class StdEnumeratorImplementation : public AbstractEnumeratorImplementation<referenceT> {

private:

	iteratorT m_begin, m_end, m_current;
	bool m_beforeBegin;

public:

	StdEnumeratorImplementation(iteratorT begin, iteratorT end) :
		m_begin(begin), m_end(end), m_current(begin), m_beforeBegin(true)
	{
	}

	StdEnumeratorImplementation(StdEnumeratorImplementation<referenceT, iteratorT> const &other) :
		m_begin(other.m_begin), m_end(other.m_end), m_current(other.m_current), m_beforeBegin(other.m_beforeBegin)
	{
	}

	StdEnumeratorImplementation<referenceT, iteratorT> operator=(StdEnumeratorImplementation<referenceT, iteratorT> const &other)
	{
		m_begin = other.m_begin;
		m_end = other.m_end;
		m_current = other.m_current;
		m_beforeBegin = other.m_beforeBegin;
	}

	virtual std::unique_ptr<AbstractEnumeratorImplementation<referenceT>> Clone() const
	{
		return std::make_unique<StdEnumeratorImplementation<referenceT, iteratorT>>(*this);
	}

	virtual referenceT GetCurrent() const override
	{
		assert(!m_beforeBegin);
		return StdContainerElementTypeCast<referenceT>(*m_current);
	}

	virtual bool MoveNext() override
	{
		if (m_current == m_end)
			return false;

		if (m_beforeBegin)
			m_beforeBegin = false;
		else
			++m_current;

		return m_current != m_end;
	}

	virtual void Reset() override
	{
		m_beforeBegin = true;
		m_current = m_begin;
	}
};

} // namespace backend
} // namespace utility
} // namespace oddf
