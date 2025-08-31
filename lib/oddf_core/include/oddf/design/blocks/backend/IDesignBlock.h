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

    Defines the `IDesignBlock` interface, which obtains information about a
    block in an ODDF design.

*/

#pragma once

#include <oddf/IObject.h>
#include <oddf/utility/IListView.h>
#include <oddf/ResourcePath.h>

#include "DesignBlockClass.h"
#include "IDesignBlockInput.h"
#include "IDesignBlockOutput.h"

#include <string>

namespace oddf {

namespace design::blocks::backend {

/*
    Obtains information about a block in an ODDF design.
*/
class IDesignBlock : public virtual IObject {

public:

	// Gets the `ResourcePath` that identifies the block within the design.
	virtual ResourcePath GetPath() const = 0;

	// Gets the `DesignBlockClass` of the block.
	virtual DesignBlockClass GetClass() const = 0;

	// Returns a `ListView` of the inputs of the design block.
	virtual std::unique_ptr<utility::IListView<IDesignBlockInput const &>> GetInputsList() const = 0;

	// Returns a `ListView` of the outputs of the design block.
	virtual std::unique_ptr<utility::IListView<IDesignBlockOutput const &>> GetOutputsList() const = 0;
};

} // namespace design::blocks::backend

template<>
struct Iid<design::blocks::backend::IDesignBlock> {

	static constexpr Uid value = { 0x4de1f419, 0xc41e, 0x4f07, 0x9f, 0x2f, 0xfd, 0x23, 0x63, 0xed, 0xa1, 0x77 };
};

} // namespace oddf
