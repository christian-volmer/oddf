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

	Decide() provides nested if-then-else functionality as a design
	block.

*/

#pragma once

namespace dfx {
namespace blocks {

#define DECLARE_DECIDE_FUNCTION(_type_) \
	node<_type_> Decide(node<bool> const &decisionNode, node<_type_> const &trueInput, node<_type_> const &falseInput); \
	bus<_type_> Decide(node<bool> const &decisionNode, bus_access<_type_> const &trueInput, bus_access<_type_> const &falseInput); \
	node<_type_> Decide(node<bool> const &decisionNode, _type_ const &trueInput, node<_type_> const &falseInput); \
	node<_type_> Decide(node<bool> const &decisionNode, node<_type_> const &trueInput, _type_ const &falseInput); \
	node<_type_> Decide(node<bool> const &decisionNode, _type_ const &trueInput, _type_ const &falseInput); \
	bus<_type_> Decide(node<bool> const &decisionNode, bus_access<_type_> const &trueInput, node<_type_> const &falseInput); \
	bus<_type_> Decide(node<bool> const &decisionNode, node<_type_> const &trueInput, bus_access<_type_> const &falseInput); \
	bus<_type_> Decide(node<bool> const &decisionNode, _type_ const &trueInput, bus_access<_type_> const &falseInput); \
	bus<_type_> Decide(node<bool> const &decisionNode, bus_access<_type_> const &trueInput, _type_ const &falseInput);

DECLARE_DECIDE_FUNCTION(bool)
DECLARE_DECIDE_FUNCTION(double)
DECLARE_DECIDE_FUNCTION(std::int32_t)
DECLARE_DECIDE_FUNCTION(std::int64_t)
DECLARE_DECIDE_FUNCTION(dynfix)

#undef DECLARE_DECIDE_FUNCTION

// Decide function with more than one decision argument for nested if(decisionNode)-elseif(decisionNode2)-...-else constructs.

#if defined(_MSC_VER)

template<typename trueT, typename... argTs>
inline auto Decide(node<bool> const &decisionNode, trueT const &trueInput, node<bool> const &decisionNode2, argTs const &...args) -> decltype(Decide(decisionNode, trueInput, Decide(decisionNode2, args...)))
{
	return Decide(decisionNode, trueInput, Decide(decisionNode2, args...));
}

#else

template<typename trueT, typename otherT, typename... argTs>
inline auto Decide(node<bool> const &decisionNode, trueT const &trueInput, node<bool> const &decisionNode2, otherT const &otherInput, argTs const &...args)
    -> decltype(Decide(decisionNode, trueInput, otherInput)) // added this to remove a warning on gcc C++11. This might not be the whole truth though!
{
	return Decide(decisionNode, trueInput, Decide(decisionNode2, otherInput, args...));
}

#endif

}
}
