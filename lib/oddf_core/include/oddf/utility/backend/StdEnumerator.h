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

    Implementation of the `IEnumerator` template interface for standard library
    containers.

*/

#pragma once

#include "../../Exception.h"
#include "../../IEnumerator.h"
#include "../../IReferenceCountSentinel.h"

#include <type_traits>
#include <utility>
#include <functional>

namespace oddf {
namespace utility {
namespace backend {

/*
    Implements the `IEnumerator` template interface for standard library
    containers.
*/
template<
	typename transformationT,
	typename iteratorT,
	typename referenceT = std::invoke_result_t<transformationT, decltype(*std::declval<iteratorT &>())>>
class StdEnumerator : public virtual IEnumerator<referenceT> {

private:

	iteratorT m_begin, m_end, m_current;
	bool m_beforeBegin;
	transformationT m_transformation;
	IReferenceCountSentinel *m_sentinel;

public:

	using reference_type = referenceT;

	StdEnumerator(iteratorT const &begin, iteratorT const &end, transformationT proj, IReferenceCountSentinel *sentinel) :
		m_begin(begin), m_end(end), m_current(begin), m_beforeBegin(true),
		m_transformation(proj),
		m_sentinel(sentinel)
	{
		if (m_sentinel)
			m_sentinel->Increment();
	}

	StdEnumerator(StdEnumerator const &) = delete;
	StdEnumerator(StdEnumerator &&) = delete;

	void operator=(StdEnumerator const &) = delete;
	void operator=(StdEnumerator &&) = delete;

	virtual ~StdEnumerator() override
	{
		if (m_sentinel)
			m_sentinel->Decrement();
	}

	virtual referenceT GetCurrent() const override
	{
		if (m_beforeBegin || m_current == m_end)
			throw Exception(ExceptionCode::IllegalMethodCall, "The enumerator does not point to a valid element. Call Reset() and/or MoveNext() first.");
		return std::invoke(m_transformation, *m_current);
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
