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

    Generic implementation of IConstEnumerable.

*/

#pragma once

#include "IConstEnumerable.h"

namespace oddf {
namespace utility {

template<typename containerT>
class GenericConstEnumerable : IConstEnumerable<typename containerT::value_type> {

private:

	using value_type = typename containerT::value_type;

	containerT const &m_container;

	class IteratorImplementation : public IConstIteratorImplementation<value_type> {

	private:

		typename containerT::const_iterator m_it;

	public:

		IteratorImplementation(typename containerT::const_iterator const &it) :
			m_it(it)
		{
		}

		virtual value_type const &Dereference() const override
		{
			return *m_it;
		}

		virtual void Increment() override
		{
			++m_it;
		}

		virtual bool Equals(IConstIteratorImplementation<value_type> const &other) const override
		{
			auto const &otherImplementation = dynamic_cast<IteratorImplementation const &>(other);
			return this->m_it == otherImplementation.m_it;
		}
	};

public:

	GenericConstEnumerable(containerT const &container) :
		m_container(container)
	{
	}

	ConstIterator<value_type> begin() const override
	{
		return ConstIterator<value_type>(std::make_shared<IteratorImplementation>(m_container.cbegin()));
	}

	ConstIterator<value_type> end() const override
	{
		return ConstIterator<value_type>(std::make_shared<IteratorImplementation>(m_container.cend()));
	}
};

} // namespace utility
} // namespace oddf
