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

#include "AbstractContainerViewImplementation.h"
#include "StdEnumeratorImplementation.h"
#include "StdContainerElementTypeCast.h"

#include <stdexcept>
#include <iterator>

namespace oddf::utility::backend {

template<typename referenceT, typename containerT>
class StdContainerViewImplementation : public AbstractContainerViewImplementation<referenceT> {

private:

	containerT &m_container;

public:

	StdContainerViewImplementation(containerT &container) :
		m_container(container)
	{
	}

	virtual std::unique_ptr<AbstractContainerViewImplementation<referenceT>> Clone() const override
	{
		return std::make_unique<StdContainerViewImplementation<referenceT, containerT>>(m_container);
	}

	virtual Enumerator<referenceT> GetEnumerator() const override
	{
		using iteratorType = decltype(std::begin(m_container));

		return Enumerator<referenceT>(std::make_unique<StdEnumeratorImplementation<referenceT, iteratorType>>(std::begin(m_container), std::end(m_container)));
	}

	virtual size_t GetSize() const override
	{
		return std::size(m_container);
	}

	virtual referenceT GetFirst() const override
	{
		return StdContainerElementTypeCast<referenceT>(*std::begin(m_container));
	}

	virtual referenceT operator[](size_t index) const override
	{
		using iteratorType = decltype(std::begin(m_container));

		if constexpr (std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<iteratorType>::iterator_category>) {

			return StdContainerElementTypeCast<referenceT>(std::begin(m_container)[index]);
		}
		else {

			(void)index;
			throw std::runtime_error("StdContainerViewImplementation::operator[]: the container does not support random element access.");
		}
	}
};

} // namespace oddf::utility::backend
