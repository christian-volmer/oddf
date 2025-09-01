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

namespace oddf::simulator::common::backend {

class ISimulatorComponent {

public:

	virtual ~ISimulatorComponent() = default;

	// Ensures that the state of the component is valid and can safely be accessed.
	virtual void EnsureValidState() = 0;

	// Marks the state of the component as invalid.
	virtual void InvalidateState() = 0;
};

} // namespace oddf::simulator::common::backend
