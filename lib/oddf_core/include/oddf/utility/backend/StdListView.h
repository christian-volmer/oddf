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

    Implementation of the `IListView` template interface for standard library
    containers.

*/

#pragma once

#include "StdCollectionView.h"

#include "../IListView.h"

namespace oddf {
namespace utility {
namespace backend {

/*
    Implements the `IListView` template interface for standard library
    containers.
*/
template<
	typename transformationT,
	typename iteratorT,
	typename referenceT /* = std::invoke_result_t<transformationT, decltype(*std::declval<iteratorT &>())> */>
class StdListView : public virtual IListView<referenceT> {

private:

	StdCollectionView<transformationT, iteratorT, referenceT> m_containerView;

public:

	using reference_type = referenceT;

	StdListView(iteratorT const &begin, iteratorT const &end, size_t size, transformationT proj, IReferenceCountSentinel *sentinel) :
		m_containerView(begin, end, size, proj, sentinel)
	{
	}

	StdListView(StdListView const &) = delete;
	StdListView(StdListView &&) = delete;

	void operator=(StdListView const &) = delete;
	void operator=(StdListView &&) = delete;

	// Returns the number of elements in the collection.
	virtual size_t GetSize() const override
	{
		return m_containerView.GetSize();
	}

	virtual referenceT GetFirst() const override
	{
		return m_containerView.GetFirst();
	}

	virtual std::unique_ptr<IEnumerator<referenceT>> GetEnumerator() const override
	{
		return m_containerView.GetEnumerator();
	}

	virtual referenceT Item(size_t index) const
	{
		return std::invoke(m_containerView.m_transformation, m_containerView.m_begin[index]);
	}
};

} // namespace backend
} // namespace utility
} // namespace oddf
