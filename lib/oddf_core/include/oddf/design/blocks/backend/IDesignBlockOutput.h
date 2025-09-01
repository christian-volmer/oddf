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

    Defines the `IDesignBlockOutput` interface, which obtains informationen
    about an output of a design block.

*/

#pragma once

#include "../../NodeType.h"

namespace oddf::design::blocks::backend {

class IDesignBlock;

/*
    Obtains informationen about an output of a design block.
*/
class IDesignBlockOutput {

public:

	virtual ~IDesignBlockOutput() = default;

	// Returns a reference to the owning design block.
	virtual IDesignBlock const &GetOwningBlock() const = 0;

	// Returns the `NodeType` of this output.
	virtual NodeType GetNodeType() const = 0;

	// Returns the index of this output in the list of outputs of the owning
	// design block.
	virtual size_t GetIndex() const = 0;
};

} // namespace oddf::design::blocks::backend
