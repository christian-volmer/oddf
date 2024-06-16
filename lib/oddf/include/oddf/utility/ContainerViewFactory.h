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

#include "Iterator.h"
#include "CollectionView.h"
#include "ListView.h"

#include <type_traits>

namespace oddf {
namespace utility {

//
// ContainerViewFactoryHelper
//

template<typename T>
struct ContainerViewFactoryHelper {

	using elementT = std::remove_const_t<T>;

	static const T &Dereference(T const &value) { return value; }
};

template<typename T>
struct ContainerViewFactoryHelper<std::unique_ptr<T>> {

	using elementT = std::remove_const_t<T>;

	static const T &Dereference(std::unique_ptr<T> const &ptr) { return *ptr; }
};

template<typename T>
struct ContainerViewFactoryHelper<T *> {

	using elementT = std::remove_const_t<T>;

	static const T &Dereference(T const *ptr) { return *ptr; }
};



//
// ContainerViewFactory
//

template<typename containerT, typename targetT>
class ContainerViewFactory {

public:

	using container_value_type = typename containerT::value_type;
	using container_const_iterator = typename containerT::const_iterator;

	using helper = ContainerViewFactoryHelper<container_value_type>;

	using elementT = std::conditional_t<std::is_same_v<targetT, void>, typename helper::elementT, targetT>;

	class IteratorImplementation : public IIteratorImplementation<elementT> {

		container_const_iterator m_it;

	public:

		IteratorImplementation(container_const_iterator const &it) :
			m_it(it) { }

		virtual std::unique_ptr<IIteratorImplementation<elementT>> Clone() const override
		{
			return std::make_unique<IteratorImplementation>(m_it);
		}

		virtual elementT const &Dereference() const override
		{
			return helper::Dereference(*m_it);
		}

		virtual void Increment() override
		{
			++m_it;
		}

		virtual bool Equals(IIteratorImplementation<elementT> const &other) const override
		{
			auto const &other_implementation = dynamic_cast<IteratorImplementation const &>(other);
			return this->m_it == other_implementation.m_it;
		}
	};

	class CollectionViewImplementation : public ICollectionViewImplementation<elementT> {

		containerT const &m_container;

	public:

		CollectionViewImplementation(containerT const &container) :
			m_container(container) { }

		virtual Iterator<elementT> begin() const { return { std::make_unique<IteratorImplementation>(m_container.cbegin()) }; }
		virtual Iterator<elementT> end() const { return { std::make_unique<IteratorImplementation>(m_container.cend()) }; }

		virtual size_t size() const override { return m_container.size(); }
	};

	class ListViewImplementation : public IListViewImplementation<elementT> {

		containerT const &m_container;

	public:

		ListViewImplementation(containerT const &container) :
			m_container(container) { }

		virtual Iterator<elementT> begin() const { return { std::make_unique<IteratorImplementation>(m_container.cbegin()) }; }
		virtual Iterator<elementT> end() const { return { std::make_unique<IteratorImplementation>(m_container.cend()) }; }

		virtual size_t size() const override { return m_container.size(); }

		virtual elementT const &at(size_t pos) const override { return helper::Dereference(m_container[pos]); }
	};

public:

	static CollectionView<elementT> CreateCollectionView(containerT const &container)
	{
		return { std::make_shared<CollectionViewImplementation>(container) };
	}

	static ListView<elementT> CreateListView(containerT const &container)
	{
		return { std::make_shared<ListViewImplementation>(container) };
	}
};

} // namespace utility
} // namespace oddf
