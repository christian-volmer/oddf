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

    Implementation of the `ICollectionView` template interface for standard
    library containers.

*/

#pragma once

#include "StdEnumerator.h"

#include "../ICollectionView.h"
#include "../IReferenceCountSentinel.h"

#include <type_traits>
#include <utility>

namespace oddf {
namespace utility {
namespace backend {

template<
	typename transformationT,
	typename iteratorT,
	typename referenceT = std::invoke_result_t<transformationT, decltype(*std::declval<iteratorT &>())>>
class StdListView;

/*
    Implements the `ICollectionView` template interface for standard library
    containers.
*/
template<
	typename transformationT,
	typename iteratorT,
	typename referenceT = std::invoke_result_t<transformationT, decltype(*std::declval<iteratorT &>())>>
class StdCollectionView : public virtual ICollectionView<referenceT> {

private:

	iteratorT m_begin, m_end;
	size_t m_size;
	transformationT m_transformation;
	IReferenceCountSentinel *m_sentinel;

	friend class StdListView<transformationT, iteratorT, referenceT>;

public:

	using reference_type = referenceT;

	StdCollectionView(iteratorT const &begin, iteratorT const &end, size_t size, transformationT proj, IReferenceCountSentinel *sentinel) :
		m_begin(begin), m_end(end),
		m_size(size),
		m_transformation(proj),
		m_sentinel(sentinel)
	{
		if (m_sentinel)
			m_sentinel->Increment();
	}

	StdCollectionView(StdCollectionView const &) = delete;
	StdCollectionView(StdCollectionView &&) = delete;

	void operator=(StdCollectionView const &) = delete;
	void operator=(StdCollectionView &&) = delete;

	virtual ~StdCollectionView() override
	{
		if (m_sentinel)
			m_sentinel->Decrement();
	}

	virtual size_t GetSize() const override
	{
		return m_size;
	}

	virtual referenceT GetFirst() const override
	{
		return std::invoke(m_transformation, *m_begin);
	}

	virtual std::unique_ptr<IEnumerator<referenceT>> GetEnumerator() const override
	{
		return std::make_unique<backend::StdEnumerator<transformationT, iteratorT, referenceT>>(m_begin, m_end, m_transformation, m_sentinel);
	}
};

} // namespace backend
} // namespace utility
} // namespace oddf
