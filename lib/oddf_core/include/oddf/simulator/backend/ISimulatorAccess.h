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

#include <oddf/simulator/backend/IClockable.h>
#include <oddf/simulator/backend/ISimulatorNodeTreeElement.h>

#include <oddf/Uid.h>
#include <oddf/Iid.h>
#include <string>

namespace oddf::simulator::backend {

class ISimulatorAccess {

public:

	virtual ~ISimulatorAccess() = default;

	virtual void *GetNamedObjectInterface(std::string const &path, Uid const &iid) const = 0;

	template<typename T>
	T &GetNamedObjectInterface(std::string const &path) const
	{
		return *static_cast<T *>(GetNamedObjectInterface(path, Iid<T>::value));
	}

	virtual void RegisterClockable(IClockable &clockable) = 0;
	virtual void UnregisterClockable(IClockable &clockable) = 0;

	virtual ISimulatorNodeTreeElement const &GetNamedNodesRoot() const = 0;
};

} // namespace oddf::simulator::backend
