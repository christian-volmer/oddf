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

	Classes that support the generation of code (e.g., Verilog) from the
	design. The 'Properties' class allows design blocks to expose named
	properties to the generator.

*/

#pragma once

namespace dfx {
namespace generator {

class Properties {

private:

/*	struct IndexedName {

		std::string name;
		int index;
		int index2;
	};

	struct IndexedNameHash {

		std::size_t operator()(IndexedName const &k) const
		{
			return std::hash<std::string>()(k.name)
				^ 0xfc7c145b * std::hash<int>()(k.index)
				^ 0xfc62afad * std::hash<int>()(k.index2);
		}
	};

	struct IndexedNameEqual {

		bool operator()(IndexedName const &lhs, IndexedName const &rhs) const
		{
			return (lhs.index == rhs.index) && (lhs.index2 == rhs.index2) && (lhs.name == rhs.name);
		}
	};

	std::unordered_map<IndexedName, int, IndexedNameHash, IndexedNameEqual> integerProperties;
	std::unordered_map<IndexedName, std::string, IndexedNameHash, IndexedNameEqual> stringProperties;
	*/

	using IndexedName = std::tuple<std::string, int, int>;

	std::map<IndexedName, int> integerProperties;
	std::map<IndexedName, std::string> stringProperties;

public:

	void SetInt(std::string const &name, int value);
	void SetInt(std::string const &name, int index, int value);
	void SetInt(std::string const &name, int index, int index2, int value);
	int GetInt(std::string const &name) const;
	int GetInt(std::string const &name, int index) const;
	int GetInt(std::string const &name, int index, int index2) const;

	template<int N> void GetIntArray(std::string const &name, int index, int(&value)[N]) const
	{
		for (int i = 0; i < N; ++i)
			value[i] = GetInt(name, index, i);
	}

	void SetString(std::string const &name, std::string const &value);
	void SetString(std::string const &name, int index, std::string const &value);
	std::string GetString(std::string const &name) const;
	std::string GetString(std::string const &name, int index) const;

	bool operator ==(Properties const &rhs) const;
	bool operator !=(Properties const &rhs) const;

	std::size_t GetHash() const;
};

}
}
