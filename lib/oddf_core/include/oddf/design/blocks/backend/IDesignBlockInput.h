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

    Defines the `IDesignBlockInput` interface, which obtains informationen
    about an input of a design block, including connectivity information.

*/

#pragma once

namespace oddf::design::blocks::backend {

class IDesignBlock;
class IDesignBlockOutput;

/*
    Obtains informationen about an input of a design block, including
    connectivity information.
*/
class IDesignBlockInput {

public:

	virtual ~IDesignBlockInput() = default;

	// Returns a reference to the owning design block.
	virtual IDesignBlock const &GetOwningBlock() const = 0;

	// Returns the index of this input in the list of inputs of the owning
	// design block.
	virtual size_t GetIndex() const = 0;

	// Returns true if the input is connected to a driving output.
	virtual bool IsConnected() const = 0;

	// If connected, returns a reference to the output of the design block
	// driving this input. Throws if this input is unconnected.
	virtual IDesignBlockOutput const &GetDriver() const = 0;
};

} // namespace oddf::design::blocks::backend
