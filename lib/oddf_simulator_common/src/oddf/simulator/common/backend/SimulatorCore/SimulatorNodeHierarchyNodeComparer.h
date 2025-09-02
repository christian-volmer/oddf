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

#include "../SimulatorCore.h"

namespace oddf::simulator::common::backend {

struct SimulatorCore::SimulatorNodeHierarchyNodeComparer {

	using is_transparent = std::true_type;

	bool operator()(std::unique_ptr<SimulatorCore::SimulatorNodeHierarchyNode> const &lhs, std::string const &rhs) const;
	bool operator()(std::string const &lhs, std::unique_ptr<SimulatorCore::SimulatorNodeHierarchyNode> const &rhs) const;
	bool operator()(std::unique_ptr<SimulatorCore::SimulatorNodeHierarchyNode> const &lhs, std::unique_ptr<SimulatorCore::SimulatorNodeHierarchyNode> const &rhs) const;
};

} // namespace oddf::simulator::common::backend
