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

#include <oddf/design/NodeType.h>

#include <memory>

namespace oddf::simulator::common::backend::blocks {

class DelayElement {

private:

	design::NodeType m_nodeType;
	void const *m_source;
	size_t m_dataSize;
	std::unique_ptr<unsigned char[]> m_state;

	friend class DelayObject;

	DelayElement(design::NodeType const &nodeType);

	void Clock();

public:

	DelayElement(DelayElement const &) = delete;
	void operator=(DelayElement const &) = delete;

	template<typename simulatorT>
	typename simulatorT::DataType const *GetStateData() const
	{
		return reinterpret_cast<typename simulatorT::DataType const *>(m_state.get());
	}

	template<typename simulatorT>
	void SetSource(simulatorT const *source)
	{
		m_source = source->GetData();
	}
};

} // namespace oddf::simulator::common::backend::blocks
