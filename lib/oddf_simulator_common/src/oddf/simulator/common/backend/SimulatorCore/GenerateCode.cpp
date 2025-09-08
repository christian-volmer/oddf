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

#include "../SimulatorCore.h"
#include "../SimulatorBlockBase/Internals.h"

#include <oddf/Exception.h>

#include <cassert>
#include <vector>
#include <iostream>

namespace oddf::simulator::common::backend {

void SimulatorCore::GenerateCode()
{
	class CodeGenerationContext : public virtual ISimulatorCodeGenerationContext {

	private:

		// Reference to the component managed by this instance.
		SimulatorComponent &m_component;

		std::vector<char> &m_code;

		SimulatorBlockBase *m_currentBlock;
		SimulatorInstruction *m_previousInstruction;
		SimulatorInstruction *m_currentInstruction;
		bool m_currentInstructionCommitted;

		CodeGenerationContext(CodeGenerationContext const &) = delete;
		void operator=(CodeGenerationContext const &) = delete;

		void ResizeCode(size_t newSize)
		{
			if (newSize < m_code.size())
				throw Exception(ExceptionCode::InvalidArgument, "Parameter 'newSize' must be greater than or equal to the current size.");

			if (m_currentInstructionCommitted)
				throw Exception(ExceptionCode::IllegalMethodCall, "Cannot do changes to code size after the current instruction has been committed.");

			if (newSize > m_code.size()) {

				auto *oldCodePointer = m_code.data();
				m_code.resize(newSize);

				ptrdiff_t offset = m_code.data() - oldCodePointer;

				if (m_previousInstruction) {

					m_previousInstruction = reinterpret_cast<SimulatorInstruction *>(
						reinterpret_cast<char *>(m_previousInstruction) + offset);
				}

				if (m_currentInstruction) {

					m_currentInstruction = reinterpret_cast<SimulatorInstruction *>(
						reinterpret_cast<char *>(m_currentInstruction) + offset);
				}
			}
		}

		virtual void *InternalStartInstruction(size_t size, size_t alignment, SimulatorInstructionFunction<> *instructionFunction) override
		{
			if (m_currentInstruction) {

				if (!m_currentInstructionCommitted)
					throw Exception(ExceptionCode::IllegalMethodCall, "Must first commit the current instruction before starting a new one.");

				m_previousInstruction = m_currentInstruction;
				m_currentInstruction = nullptr;
				m_currentInstructionCommitted = false;
			}

			auto misalignment = m_code.size() % alignment;
			if (misalignment)
				ResizeCode(m_code.size() + alignment - misalignment);

			if (m_previousInstruction)
				m_previousInstruction->m_size = m_code.data() + m_code.size() - reinterpret_cast<char *>(m_previousInstruction);

			ResizeCode(m_code.size() + size);

			m_currentInstruction = reinterpret_cast<SimulatorInstruction *>(m_code.data() + m_code.size() - size);
			m_currentInstruction->m_size = 0;
			m_currentInstruction->m_function = instructionFunction;

			return m_currentInstruction;
		}

		virtual size_t InternalAddRecord(size_t size, size_t alignment) override
		{
			if (!m_currentInstruction || m_currentInstructionCommitted)
				throw Exception(ExceptionCode::IllegalMethodCall, "Must first start an instruction before calling this method.");

			auto misalignment = m_code.size() % alignment;
			if (misalignment)
				ResizeCode(m_code.size() + alignment - misalignment);

			ResizeCode(m_code.size() + size);

			return m_code.data() + m_code.size() - reinterpret_cast<char *>(m_currentInstruction) - size;
		}

		virtual void *InternalCommitInstruction() override
		{
			if (!m_currentInstruction)
				throw Exception(ExceptionCode::IllegalMethodCall, "There is no instruction to commit.");

			if (m_currentInstructionCommitted)
				throw Exception(ExceptionCode::IllegalMethodCall, "The current instruction has already been committed.");

			m_currentInstructionCommitted = true;
			return m_currentInstruction;
		}

		virtual void *InternalGetRecord(size_t offset) override
		{
			if (!m_currentInstruction)
				throw Exception(ExceptionCode::IllegalMethodCall, "Cannot not call this function unless the current instruction has been committed.");

			// TODO check if `offset` is within the current instruction

			return reinterpret_cast<void *>(reinterpret_cast<char *>(m_currentInstruction) + offset);
		}

		//
		// Input binding
		//

		void InternalRegisterInput(size_t index, void const **inputPointerPointer, design::NodeType::TypeId expectedTypeId)
		{
			if (!m_currentInstruction || !m_currentInstructionCommitted)
				throw Exception(ExceptionCode::IllegalMethodCall, "Cannot not call this function unless the current instruction has been committed.");

			auto &inputs = m_currentBlock->m_internals->m_inputs;

			if (index >= inputs.size())
				throw Exception(ExceptionCode::InvalidArgument, "Parameter 'index' is out of range.");

			auto &input = inputs[index];

			if (input.m_inputPointerReference)
				throw Exception(ExceptionCode::IllegalMethodCall, "Function was already called on this input. Cannot call it a second time.");

			if (input.GetType().GetTypeId() != expectedTypeId)
				throw Exception(ExceptionCode::InvalidArgument, "Type of the argument must match the type of the simulator block input.");

			// TODO: confirm that `inputPointerPointer` is within the bounds of the current instruction.

			input.m_inputPointerReference = reinterpret_cast<char const *>(inputPointerPointer) - m_code.data();
		}

		virtual void BindInputReference(size_t index, types::Boolean const *&inputPointerReference) override
		{
			InternalRegisterInput(index, reinterpret_cast<void const **>(&inputPointerReference), design::NodeType::BOOLEAN);
		}

		virtual void BindInputReference(size_t index, types::FixedPoint const *&inputPointerReference) override
		{
			InternalRegisterInput(index, reinterpret_cast<void const **>(&inputPointerReference), design::NodeType::FIXED_POINT);
		}

		//
		// Output binding
		//

		SimulatorBlockOutput const &InternalRegisterOutput(size_t index, void *storagePointer, design::NodeType::TypeId expectedTypeId)
		{
			if (!m_currentInstruction || !m_currentInstructionCommitted)
				throw Exception(ExceptionCode::IllegalMethodCall, "Cannot not call this function unless the current instruction has been committed.");

			auto &outputs = m_currentBlock->m_internals->m_outputs;

			if (index >= outputs.size())
				throw Exception(ExceptionCode::InvalidArgument, "Parameter 'index' is out of range.");

			auto &output = outputs[index];

			if (output.m_storageReference)
				throw Exception(ExceptionCode::IllegalMethodCall, "Function was already called on this output. Cannot call it a second time.");

			if (output.GetType().GetTypeId() != expectedTypeId)
				throw Exception(ExceptionCode::InvalidArgument, "Type of the argument must match the type of the simulator block output.");

			// TODO: confirm that `storagePointer` is within the bounds of the current instruction.

			output.m_storageReference = reinterpret_cast<char const *>(storagePointer) - m_code.data();

			return output;
		}

		virtual void BindOutput(size_t index, types::Boolean &outputReference) override
		{
			InternalRegisterOutput(index, &outputReference, design::NodeType::BOOLEAN);
		}

		virtual void BindOutput(size_t index, types::FixedPoint &outputReference) override
		{
			auto &output = InternalRegisterOutput(index, &outputReference, design::NodeType::FIXED_POINT);

			if (types::FixedPoint::GetElementCount(output.GetType()) != outputReference.m_length)
				throw Exception(ExceptionCode::InvalidArgument, "The length (number of elements) of the given 'outputReference' does not match the type of the output.");
		}

		void TranslateOutputReferences()
		{
			for (auto *block : m_component.m_blocks) {

				for (auto &output : block->m_internals->m_outputs)
					output.m_storagePointer = m_code.data() + output.m_storageReference;
			}
		}

		void TranslateInputReferences()
		{
			for (auto *block : m_component.m_blocks) {

				for (auto &input : block->m_internals->m_inputs) {

					if (input.m_inputPointerReference)
						*reinterpret_cast<void const **>(m_code.data() + input.m_inputPointerReference) = input.m_driver->m_storagePointer;
				}
			}
		}

		//
		// ISimulatorComponentContext
		//

		virtual void RegisterGlobalObject(std::string name, std::unique_ptr<IObject> &&object) override
		{
			m_component.m_simulatorCore.RegisterGlobalObject(name, std::move(object));
		}

		virtual ISimulatorComponent &GetCurrentComponent() noexcept override
		{
			return m_component;
		}

		virtual void RegisterComponentObject(Uid const &clsid, std::unique_ptr<IObject> &&object) override
		{
			m_component.RegisterComponentObject(clsid, std::move(object));
		}

		virtual void *GetComponentObject(Uid const &clsid, Uid const &iid) const override
		{
			return m_component.GetComponentObject(clsid, iid);
		}

	public:

		CodeGenerationContext(SimulatorComponent &component) :
			m_component(component),
			m_code(component.m_code),
			m_currentBlock(nullptr),
			m_previousInstruction(nullptr),
			m_currentInstruction(nullptr),
			m_currentInstructionCommitted(false)
		{
		}

		void GenerateCode()
		{
			for (auto *block : m_component.m_blocks) {

				assert(block);
				m_currentBlock = block;

				std::cout << "  '" << block->GetDesignPathHint() << "'\n";

				if (m_currentInstruction) {

					assert(m_currentInstructionCommitted);

					m_previousInstruction = m_currentInstruction;
					m_currentInstruction = nullptr;
					m_currentInstructionCommitted = false;
				}

				m_currentBlock->GenerateCode(*this);

				if (m_currentInstruction && !m_currentInstructionCommitted)
					throw Exception(ExceptionCode::Unexpected, "Must call CommitInstruction() after call to StartInstruction().");

				// If a block has outputs it must somehow generate code and provide an address to the value
				// of that output, because other inputs will refer to that address.
				for (auto const &output : m_currentBlock->m_internals->m_outputs)
					if (!output.m_storageReference)
						throw Exception(ExceptionCode::Unexpected, "Block failed to register at least one of its outputs.");
			}

			m_code.shrink_to_fit();

			TranslateOutputReferences();
			TranslateInputReferences();

			auto *currentInstruction = reinterpret_cast<SimulatorInstruction *>(m_code.data());

			while (currentInstruction) {

				if (currentInstruction->m_size)
					currentInstruction->m_next = reinterpret_cast<SimulatorInstruction *>(reinterpret_cast<char *>(currentInstruction) + currentInstruction->m_size);
				else
					currentInstruction->m_next = nullptr;

				currentInstruction = currentInstruction->m_next;
			}
		}
	};

	size_t componentIndex = 0;
	for (auto &component : m_components) {

		std::cout << "Generating code for component " << componentIndex << "\n";
		auto codeGenerationContext = CodeGenerationContext(component);
		codeGenerationContext.GenerateCode();

		std::cout << "  Code size = " << component.m_code.size() << " bytes\n\n";

		++componentIndex;
	}
}

} // namespace oddf::simulator::common::backend
