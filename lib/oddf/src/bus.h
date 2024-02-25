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

	Classes for the support of busses (arrays of nodes) in the design.

*/

#pragma once

#include "node.h"
#include "messages.h"

namespace dfx {

template<typename T> class bus;
template<typename T> class forward_bus;

template<typename T>
class bus_access {

protected:

	std::vector<node<T>> *NodesPointer;

	int First;
	int Step;
	int Width;

	static int resolve_index(int width, int index)
	{
		// TODO: use static global function
		if (index >= 1 && index <= width + 1)
			return index;
		else if (-index >= 1 && -index <= width + 1)
			return width + 1 + index;
		else
			throw design_error("Bus index out of range.");
	}

	bus_access(std::vector<node<T>> *nodesPointer, int first, int step, int width) :
		NodesPointer(nodesPointer),
		First(first),
		Step(step),
		Width(width)
	{
	}

	bus_access(bus_access<T> const &) = delete;

	bus_access(bus_access<T> &&other) :
		NodesPointer(other.NodesPointer),
		First(other.First),
		Step(other.Step),
		Width(other.Width)
	{
		other.NodesPointer = nullptr;
		other.First = 0;
		other.Step = 0;
		other.Width = 0;
	}

	friend class bus<T>;
	friend class forward_bus<T>;

public:

	// The number of elements of the bus.
	int width() const
	{
		return Width;
	}

	// One-based access
	node<T> &operator ()(int index)
	{
		index = resolve_index(width(), index);
		if (index < 1)
			throw design_error("Bus index out of range.");

		return NodesPointer->at(First + (index - 1) * Step);
	}

	// One-based const access
	node<T> const &operator ()(int index) const
	{
		index = resolve_index(width(), index);
		if (index < 1)
			throw design_error("Bus index out of range.");

		return NodesPointer->at(First + (index - 1) * Step);
	}

	// Zero-based access
	node<T> &operator [](int index)
	{
		return NodesPointer->at(First + index * Step);
	}

	// Zero-based const access
	node<T> const &operator [](int index) const
	{
		return NodesPointer->at(First + index * Step);
	}

	// Returns the given span of elements. Indices are one-based. Negative indices count from the end of the bus.
	bus_access<T> span(int first, int last, int step = 1)
	{
		first = resolve_index(Width, first);
		last = resolve_index(Width, last);

		int width;

		if (step != 0)
			width = (last - first + step) / step;
		else
			throw design_error("Step argument in span specification must be different from zero.");

		if (width < 0)
			throw design_error("Invalid span specification. Length of resulting bus would be negative.");

		// TODO: do additional range checking here...
		return bus_access<T>(NodesPointer, First + (first - 1) * Step, Step * step, width);
	}

	// Returns the given span of elements. Indices are one-based. Negative indices count from the end of the bus.
	bus<T> span(int first, int last, int step = 1) const
	{
		first = resolve_index(Width, first);
		last = resolve_index(Width, last);

		int width;

		if (step != 0)
			width = (last - first + step) / step;
		else
			throw design_error("Step argument in span specification must be different from zero.");

		if (width < 0)
			throw design_error("Invalid span specification. Length of resulting bus would be negative.");

		// TODO: do additional range checking here...
		return bus_access<T>(NodesPointer, First + (first - 1) * Step, Step * step, width);
	}

	// Returns the first element of the bus.
	node<T> &first()
	{
		return operator()(1);
	}

	// Returns the first element of the bus.
	node<T> first() const
	{
		return operator()(1);
	}

	// Returns the last element of the bus.
	node<T> &last()
	{
		return operator()(-1);
	}

	// Returns the last element of the bus.
	node<T> last() const
	{
		return operator()(-1);
	}

	// Reverses the order of the elements.
	bus_access<T> reverse()
	{
		return span(-1, 1, -1);
	}

	// Reverses the order of the elements.
	bus<T> reverse() const
	{
		return span(-1, 1, -1);
	}

	// Returns all elements but the last.
	bus_access<T> most()
	{
		return span(1, -2);
	}

	// Returns all elements but the last.
	bus<T> most() const
	{
		return span(1, -2);
	}

	// Returns all elements but the first.
	bus_access<T> rest()
	{
		return span(2, -1);
	}

	// Returns all elements but the first.
	bus<T> rest() const
	{
		return span(2, -1);
	}

	// 'length' >= 1: returns the first 'length' elements from the bus. 'length' <= -1: return the last '-length' elements from the bus.
	bus_access<T> take(int length)
	{
		if (length >= 0)
			return span(1, length);
		else
			return span(length, -1);
	}

	// 'length' >= 1: returns the first 'length' elements from the bus. 'length' <= -1: return the last '-length' elements from the bus.
	bus<T> take(int length) const
	{
		if (length >= 0)
			return span(1, length);
		else
			return span(length, -1);
	}

	// 'count' >= 1: returns the bus with the first 'count' elements dropped. 'count' <= -1: return the bus with the last '-count' elements dropped.
	bus_access<T> drop(int count)
	{
		if (count >= 0)
			return span(count + 1, -1);
		else
			return span(1, -1 + count);
	}

	// 'count' >= 1: returns the bus with the first 'count' elements dropped. 'count' <= -1: return the bus with the last '-count' elements dropped.
	bus<T> drop(int count) const
	{
		if (count >= 0)
			return span(count + 1, -1);
		else
			return span(1, -1 + count);
	}

	bus_access<T> &operator =(bus_access<T> const &other)
	{
		if (Width != other.Width)
			throw design_error("Assigning to sub-range of a bus: 'other' must have same width as 'this'");

		if (other.NodesPointer == this->NodesPointer) {

			bus<T> otherCopy(other);
			return this->operator=(otherCopy);
		}

		for (int i = 1; i <= Width; ++i)
			this->operator()(i) = other(i);

		return *this;
	}
};


//
// bus<T>
//

template<typename T>
class bus : public bus_access<T> {

protected:

	using bus_access<T>::Width;

	std::vector<node<T>> Nodes;

public:

	bus() : bus_access<T>(&Nodes, 0, 1, 0), Nodes()
	{
	}

	bus(bus_access<T> const &other) : bus_access<T>(&Nodes, 0, 1, 0), Nodes()
	{
		for (int i = 1; i <= other.width(); ++i)
			append(other(i));
	}

	bus(bus<T> const &other) : bus_access<T>(&Nodes, 0, 1, 0), Nodes()
	{
		for (int i = 1; i <= other.width(); ++i)
			append(other(i));
	}

	bus(bus<T> &&other) : bus_access<T>(&Nodes, 0, 1, other.Width), Nodes(std::move(other.Nodes))
	{
	}

	bus(node<T> const &node, int width) : bus_access<T>(&Nodes, 0, 1, 0), Nodes()
	{
		for (int i = 1; i <= width; ++i)
			append(node);
	}

	explicit bus(node<T> const &node) : bus_access<T>(&Nodes, 0, 1, 0), Nodes()
	{
		append(node);
	}

	explicit bus(int width) : bus_access<T>(&Nodes, 0, 1, 0), Nodes()
	{
		for (int i = 1; i <= width; ++i)
			append(node<T>());
	}

	node<T> &append(node<T> const &node)
	{
		Nodes.push_back(node);
		++Width;
		return Nodes.back();
	}

	node<T> append(node<T> &&node)
	{
		Nodes.push_back(std::move(node));
		++Width;
		return Nodes.back();
	}

	void clear()
	{
		Nodes.clear();
		Width = 0;
	}

	void append(bus_access<T> const &bus)
	{
		for (int i = 1; i <= bus.width(); ++i)
			append(bus(i));
	}

	bus<T> &operator =(bus_access<T> const &other)
	{
		if (other.NodesPointer == this->NodesPointer) {

			bus<T> otherCopy(other);
			return this->operator=(otherCopy);
		}
		else {

			Nodes.clear();
			Width = 0;

			for (int i = 1; i <= other.width(); ++i)
				append(other(i));

			return *this;
		}
	}

	bus<T> &operator =(bus<T> const &other)
	{
		return this->operator=(static_cast<bus_access<T> const &>(other));
	}
};


//
// forward_bus<T>
//

template<typename T> class forward_bus : public bus<typename types::TypeTraits<T>::internalType> {

	// TODO: ugly hack? See comment for forward_node<T>

	using internalType = typename types::TypeTraits<T>::internalType;

protected:

	using bus<internalType>::Width;
	using bus<internalType>::Nodes;

public:

	forward_bus() : bus<internalType>() {}
	forward_bus(forward_bus<T> const &) = delete;
	forward_bus(forward_bus<T> &&other) : bus<internalType>(std::move(other)) {}

//	explicit forward_bus(int width) : bus<internalType>(width) {}

	explicit forward_bus(int width) : bus<internalType>()
	{
		create(width);
	}

	explicit forward_bus(int width, internalType const &templateType) : bus<internalType>()
	{
		create(width, templateType);
	}

	void create(int width)
	{
		if (Width != 0)
			throw design_error("Can call create() only on a zero-width bus.");

		Nodes.reserve(width);
		for (int i = 1; i <= width; ++i)
			append();
	}

	void create(int width, internalType const &templateType)
	{
		if (Width != 0)
			throw design_error("Can call create() only on a zero-width bus.");

		Nodes.reserve(width);
		for (int i = 1; i <= width; ++i)
			append(templateType);
	}

	node<internalType> append()
	{
		Nodes.emplace_back(forward_node<T>());
		++Width;
		return Nodes.back();
	}

	node<internalType> append(internalType const &templateType)
	{
		Nodes.emplace_back(forward_node<T>(templateType));
		++Width;
		return Nodes.back();
	}

	bus<internalType> append(int length)
	{
		bus<internalType> result;
		while (length-- > 0)
			result.append(append());
		return result;
	}

	void operator <<=(bus_access<internalType> const &other)
	{
		if (Width != other.width())
			throw design_error("Assigning to sub-range of a bus by using \"<<=\": 'other' must have same width as 'this'");

		// forward_bus is a friend of node, so it can access the private <<= operator
		for (int i = 1; i <= Width; ++i)
			this->operator()(i) <<= other(i);
	}

	// Hide the = operator to avoid calling it accidentally.
	forward_bus<T> &operator =(forward_bus<T> const &other) = delete;
	forward_bus<T> &operator =(bus_access<internalType> const &other) = delete;
	forward_bus<T> &operator =(bus<internalType> const &other) = delete;
};


template<typename T>
inline bus<T> ExpandToBus(node<T> const &node, int width)
{
	return bus<T>(node, width);
}

template<typename T>
inline bus<T> join(bus_access<T> const &a, bus_access<T> const &b)
{
	// TODO: generalise to arbitrary mixture of busses and nodes.
	bus<T> outputBus;
	outputBus.append(a);
	outputBus.append(b);
	return outputBus;
}

template<typename T>
inline bus<T> join(bus_access<T> const &a, bus_access<T> const &b, bus_access<T> const &c)
{
	bus<T> outputBus;
	outputBus.append(a);
	outputBus.append(b);
	outputBus.append(c);
	return outputBus;
}

template<typename T>
inline bus<T> join(node<T> const &a, bus_access<T> const &b)
{
	bus<T> outputBus;
	outputBus.append(a);
	outputBus.append(b);
	return outputBus;
}

template<typename T>
inline bus<T> join(node<T> const &a, node<T> const &b, bus_access<T> const &c)
{
	bus<T> outputBus;
	outputBus.append(a);
	outputBus.append(b);
	outputBus.append(c);
	return outputBus;
}

template<typename T>
inline bus<T> join(bus_access<T> const &a, node<T> const &b)
{
	bus<T> outputBus;
	outputBus.append(a);
	outputBus.append(b);
	return outputBus;
}

template<typename T>
inline bus<T> join(node<T> const &a, node<T> const &b)
{
	bus<T> outputBus;
	outputBus.append(a);
	outputBus.append(b);
	return outputBus;
}

template<typename T>
inline bus<T> join(node<T> const &a, node<T> const &b, node<T> const &c, node<T> const &d)
{
	bus<T> outputBus;
	outputBus.append(a);
	outputBus.append(b);
	outputBus.append(c);
	outputBus.append(d);
	return outputBus;
}

template<typename T>
inline bus<T> join(bus_access<T> const &a, node<T> const &b, bus_access<T> const &c)
{
	bus<T> outputBus;
	outputBus.append(a);
	outputBus.append(b);
	outputBus.append(c);
	return outputBus;
}

template<typename T>
inline bus<T> riffle(bus_access<T> const &a, bus_access<T> const &b)
{
	if (a.width() != b.width())
		throw design_error("Busses must have same width in riffle operation.");

	bus<T> outputBus;

	for (int i = 1; i <= a.width(); ++i) {
		outputBus.append(a(i));
		outputBus.append(b(i));
	}

	return outputBus;
}

}
