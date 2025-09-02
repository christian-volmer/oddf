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

#include <oddf/simulator/common/backend/SimulatorBlockBase.h>

#include <oddf/utility/MakeContainerView.h>

#include <oddf/Exception.h>

namespace oddf::simulator::common::backend {

SimulatorBlockOutput::SimulatorBlockOutput(SimulatorBlockBase &owningBlock, design::NodeType const &nodeType, size_t index) :
	m_owningBlock(owningBlock),
	m_nodeType(nodeType),
	m_index(index),
	m_targets(),
	m_storageReference(),
	m_storagePointer()
{
}

SimulatorBlockOutput::SimulatorBlockOutput(SimulatorBlockOutput &&other) :
	m_owningBlock(other.m_owningBlock),
	m_nodeType(),
	m_index(0),
	m_targets(),
	m_storageReference(),
	m_storagePointer()
{
	// Since the address of an output object must never change, it must not be moved.
	throw oddf::Exception(oddf::ExceptionCode::Unexpected);
}

design::NodeType SimulatorBlockOutput::GetType() const noexcept
{
	return m_nodeType;
}

template<>
void const *SimulatorBlockOutput::GetPointer<void>() const
{
	return reinterpret_cast<void const *>(m_storagePointer);
}

template<>
types::Boolean const *SimulatorBlockOutput::GetPointer<types::Boolean>() const
{
	if (GetType().GetTypeId() == design::NodeType::BOOLEAN)
		return reinterpret_cast<types::Boolean const *>(m_storagePointer);
	else
		throw Exception(ExceptionCode::InvalidArgument, "Type argument `T` (types::Boolean) does not match the type of the output.");
}

template<>
types::FixedPoint const *SimulatorBlockOutput::GetPointer<types::FixedPoint>() const
{
	if (GetType().GetTypeId() == design::NodeType::FIXED_POINT)
		return reinterpret_cast<types::FixedPoint const *>(m_storagePointer);
	else
		throw Exception(ExceptionCode::InvalidArgument, "Type argument `T` (types::FixedPoint) does not match the type of the output.");
}

size_t SimulatorBlockOutput::GetIndex() const noexcept
{
	return m_index;
}

SimulatorBlockBase const &SimulatorBlockOutput::GetOwningBlock() const noexcept
{
	return m_owningBlock;
}

std::unique_ptr<ICollectionView<SimulatorBlockInput const &>> SimulatorBlockOutput::GetTargetsCollection() const
{
	return utility::MakeContainerView(m_targets, [](auto &e) -> SimulatorBlockInput const & { return *e; });
}

bool SimulatorBlockOutput::HasConnections() const noexcept
{
	return !m_targets.empty();
}

} // namespace oddf::simulator::common::backend
