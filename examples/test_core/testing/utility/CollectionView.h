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

#include <oddf/utility/CollectionView.h>

#include "../Expect.h"

#include <iostream>
#include <memory>
#include <type_traits>

namespace oddf {
namespace testing {
namespace utility {

void Test_CollectionView();

template<template<typename> class containerTemplate>
inline void TestCollectionViewForStdContainer()
{
	struct Base {

		virtual int GetValue() const = 0;
		virtual ~Base() { }
	};

	struct Derived : public Base {

	private:

		int m_Value;

	public:

		Derived(int value) :
			m_Value(value) { }

		virtual int GetValue() const override { return m_Value; }
	};

	// We use a unique_ptr of a derived class as the element type because we want to
	// test the various type casts that are supported by our container-view classes
	using elementT = std::unique_ptr<Derived>;

	// These are our source elements. We put them in a plain array because this allows inplace construction.
	elementT src[5] = {

		std::make_unique<Derived>(6),
		std::make_unique<Derived>(4),
		std::make_unique<Derived>(5),
		std::make_unique<Derived>(3),
		std::make_unique<Derived>(2)
	};

	// Create empty container object of given template type.
	using containerT = containerTemplate<elementT>;
	containerT container;

	// Move elements to the container
	std::move(std::begin(src), std::end(src), std::inserter(container, std::end(container)));

	//
	// No type conversion on the element type
	//

	// Create default CollectionView
	auto collectionView = oddf::utility::MakeCollectionView(container);

	// Get an Enumerator from the CollectionView
	auto enumerator = collectionView.GetEnumerator();

	// Confirm the element type returned by the Enumerator
	static_assert(std::is_same_v<decltype(enumerator.GetCurrent()), decltype(*container.begin())>);

	// Create a CollectionView where the elements of type unique_ptr<Derived> are converted to a plain pointer to const Base.
	auto collectionViewToConstBasePointer = oddf::utility::MakeCollectionView<Base const *>(container);
	auto enumeratorToConstBasePointer = collectionViewToConstBasePointer.GetEnumerator();

	// Confirm the element type returned by the Enumerator
	static_assert(std::is_same_v<decltype(enumeratorToConstBasePointer.GetCurrent()), Base const *>);

	// Create a CollectionView where the elements of type unique_ptr<Derived> are converted to a reference
	auto collectionViewToReference = oddf::utility::MakeCollectionView<Derived &>(container);
	auto enumeratorToReference = collectionViewToReference.GetEnumerator();

	// Confirm the element type returned by the Enumerator
	static_assert(std::is_same_v<decltype(enumeratorToReference.GetCurrent()), Derived &>);

	Expect(collectionView.GetSize() == 5);
	Expect(collectionViewToConstBasePointer.GetSize() == 5);
	Expect(collectionViewToReference.GetSize() == 5);

	/*

	TODO: should not be required anymore.

	enumerator.Reset();
	enumeratorToConstBasePointer.Reset();
	enumeratorToReference.Reset();

	*/

	size_t i = 0;

	while (enumerator.MoveNext()) {

		enumeratorToConstBasePointer.MoveNext();
		enumeratorToReference.MoveNext();
		++i;

		Expect(enumerator.GetCurrent()->GetValue() == enumeratorToConstBasePointer.GetCurrent()->GetValue());
		Expect(enumerator.GetCurrent()->GetValue() == enumeratorToReference.GetCurrent().GetValue());
	}

	Expect(enumeratorToConstBasePointer.MoveNext() == false);
	Expect(enumeratorToReference.MoveNext() == false);
	Expect(i == collectionView.GetSize());
}

} // namespace utility
} // namespace testing
} // namespace oddf
