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

#include <oddf/utility/ListView.h>

#include "IBlockInput.h"
#include "IBlockOutput.h"

#include <string>

namespace oddf {
namespace design {
namespace backend {

class DesignBlockClass {

private:

	std::string m_className;

public:

	DesignBlockClass(std::string const &className) :
		m_className(className) { }

	friend bool operator<(DesignBlockClass const &lhs, DesignBlockClass const &rhs)
	{
		return lhs.m_className < rhs.m_className;
	}

	std::string ToString() const { return m_className; };
};

class IDesignBlock {

protected:

	IDesignBlock() = default;
	virtual ~IDesignBlock() { }

public:

	IDesignBlock(IDesignBlock const &) = delete;

	virtual std::string GetBlockPath() const = 0;
	virtual DesignBlockClass GetClass() const = 0;

	virtual utility::ListView<IBlockInput> GetInputsList() const = 0;
	virtual utility::ListView<IBlockOutput> GetOutputsList() const = 0;
};

} // namespace backend
} // namespace design
} // namespace oddf
