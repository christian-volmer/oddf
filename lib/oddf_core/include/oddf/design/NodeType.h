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

#include <string>

namespace oddf::design {

class NodeType final {

public:

	enum TypeId {

		UNDEFINED = 0,
		BOOLEAN = 1,
		INTEGER = 2,
		REAL = 3,
		BIT_VECTOR = 4,
		FIXED_POINT = 5
	};

private:

	TypeId m_type;
	int m_param1, m_param2;

	NodeType(TypeId type, int param1 = 0, int param2 = 0);

public:

	NodeType();

	static NodeType Undefined() noexcept;
	static NodeType Boolean() noexcept;
	static NodeType Integer() noexcept;
	static NodeType Real() noexcept;
	static NodeType BitVector(int size);
	static NodeType FixedPoint(bool signedness, int size, int fraction);

	std::string ToString() const;

	TypeId GetTypeId() const noexcept;

	bool IsSigned() const noexcept;
	bool IsUnsigned() const noexcept;

	int GetWordWidth() const noexcept;
	int GetFraction() const noexcept;

	bool IsDefined() const noexcept;

	bool operator==(NodeType const &other) const noexcept;
	bool operator!=(NodeType const &other) const noexcept;
};

} // namespace oddf::design
