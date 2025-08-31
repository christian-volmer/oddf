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
#include "NamedNodeComparer.h"

#include <oddf/simulator/backend/ISimulatorNodeTreeElement.h>
#include <oddf/utility/ICollectionView.h>

namespace oddf::simulator::common::backend {

struct SimulatorCore::NamedNode : public virtual simulator::backend::ISimulatorNodeTreeElement {

	std::string m_name;
	oddf::design::NodeType m_type;
	void const *m_pointer;
	std::set<NamedNode, NamedNodeComparer> m_children;

	NamedNode(std::string const &name); /* :
	     m_name(name),
	     m_type(),
	     m_pointer(),
	     m_children()
	 {
	 }*/

	NamedNode(NamedNode const &) = delete;
	void operator=(NamedNode const &) = delete;

	NamedNode(NamedNode &&) = default;
	NamedNode &operator=(NamedNode &&) = default;

	virtual std::string GetName() const override;
	virtual bool IsNode() const override;
	virtual design::NodeType GetType() const override;
	virtual void Read(void * /*buffer */, size_t /* count */) const override;

	virtual std::unique_ptr<utility::ICollectionView<ISimulatorNodeTreeElement const &>> GetChildren() const override;

	virtual void *GetInterface(oddf::Uid const &iid) override;
};

} // namespace oddf::simulator::common::backend
