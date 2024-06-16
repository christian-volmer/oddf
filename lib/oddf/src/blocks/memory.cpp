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

    Memory() implements a single-clock domain two-port memory with output
    register.

*/

#include "../global.h"
#include "../generator/properties.h"
#include "constant.h"

namespace dfx {
namespace backend {
namespace blocks {

template<typename T>
class memory_block : public BlockBase, private IStep, private dfx::blocks::IMemoryBackdoor<T> {

private:

	int depth;
	int width;

	std::vector<T> content;
	std::vector<T> outputRegister;

	InputPin<bool> enableInput;
	InputPin<dynfix> rdAddressInput;
	InputPin<dynfix> wrAddressInput;
	InputPin<bool> wrDataEnable;
	std::list<InputPin<T>> wrDataInput;

	std::list<OutputPin<T>> rdDataOutput;

	T defaultValue;

	bool CanEvaluate() const override
	{
		return true;
	}

	void Evaluate() override
	{
		auto registerIt = outputRegister.cbegin();
		for (auto &output : rdDataOutput)
			types::Copy<T>(output.value, *(registerIt++));

		/*int size = (int)content.size();
	int rdaddress = rdAddressInput.GetValue().data[0]; // lazy conversion from ufix to int

	if ((rdaddress < 0) || (rdaddress >= size))
		throw design_error(string_printf(GetFullName() + ": read address input (address = %d) is beyond the size of the memory (size = %d).", rdaddress, size));

	int flatAddress = rdaddress * width;
	for (auto &output : rdDataOutput)
		types::Copy<T>(output.value, content[flatAddress++]);*/
	}

	void Step() override
	{
		if (enableInput.GetValue()) {

			int size = (int)content.size();

			//
			// Read from 'rdAddress' and put data into outputRegister
			//

			{
				int rdaddress = rdAddressInput.GetValue().data[0]; // lazy conversion from ufix to int

				if ((rdaddress < 0) || (rdaddress >= size))
					throw design_error(string_printf(GetFullName() + ": read address input (address = %d) is beyond the size of the memory (size = %d).", rdaddress, size));

				int flatAddress = rdaddress * width;
				for (int i = 0; i < width; ++i) {

					T tmp;
					types::Copy<T>(tmp, content[flatAddress + i]);
					outputRegister[i] = tmp;
				}
			}

			//
			// Write data to 'wrAddress'
			//

			if (wrDataEnable.GetValue()) {

				int wraddress = wrAddressInput.GetValue().data[0]; // lazy conversion from ufix to int

				if ((wraddress < 0) || (wraddress >= size))
					throw design_error(string_printf(GetFullName() + ": write address input (address = %d) is beyond the size of the memory (size = %d).", wraddress, size));

				int flatAddress = wraddress * width;
				for (auto const &input : wrDataInput) {

					T tmp;
					types::Copy<T>(tmp, input.GetValue());
					content[flatAddress++] = tmp;
				}
			}

			SetDirty();
		}
	}

	void AsyncReset() override
	{
		// Memories in Verilog do not have a reset.
		// for (int i = 0; i < (int)content.size(); ++i)
		// 	content[i] = defaultValue;
	}

	IStep *GetStep() override
	{
		return this;
	}

	source_blocks_t GetSourceBlocks() const override
	{
		return {};
	}

	std::string GetInputPinName(int index) const override
	{
		switch (index) {

		case 0:
			return "clkEnable";
		case 1:
			return "rdAddress";
		case 2:
			return "wrAddress";
		case 3:
			return "wrEnable";
		default:

			if (index - 4 < width)
				return "wrDataIn" + std::to_string(index - 4);
			else {

				assert(false);
				return "<ERROR>";
			}
		}
	}

	void GetProperties(dfx::generator::Properties &properties) const override
	{
		properties.SetInt("Depth", depth);
		properties.SetInt("Width", width);
	}

public:

	memory_block(int depth, node<dynfix> const &rdaddress, node<dynfix> const &wraddress, bus_access<T> const &wrdatain, node<bool> const &wrenable, dfx::blocks::IMemoryBackdoor<T> **memoryBackdoor) :
		BlockBase("memory"),
		depth(depth),
		width(wrdatain.width()),
		content(),
		outputRegister(width),
		enableInput(this, Design::GetCurrent().hasCustomDefaultEnable ? Design::GetCurrent().customDefaultEnable : dfx::blocks::Constant(true)),
		rdAddressInput(this, rdaddress),
		wrAddressInput(this, wraddress),
		wrDataEnable(this, wrenable),
		wrDataInput(),
		rdDataOutput(),
		defaultValue(types::DefaultFrom(wrdatain.first().GetDriver()->value))
	{
		auto readAddressTypeDesc = types::GetDescription(rdaddress.GetDriver()->value);
		auto writeAddressTypeDesc = types::GetDescription(wraddress.GetDriver()->value);

		if (depth <= 0)
			throw design_error(GetFullName() + ": 'depth' must be positive.");

		if (readAddressTypeDesc.GetFraction() != 0)
			throw design_error(GetFullName() + ": type of 'ReadAddress' input must have fractional equal to zero. Current type is '" + readAddressTypeDesc.ToString() + "'.");

		if (readAddressTypeDesc.GetWordWidth() > 31)
			throw design_error(GetFullName() + ": word width of 'ReadAddress' input must be less than 32. Current type is '" + readAddressTypeDesc.ToString() + "'.");

		if (readAddressTypeDesc != writeAddressTypeDesc)
			throw design_error(GetFullName() + ": the types of the 'ReadAddress' input ('" + readAddressTypeDesc.ToString() + "') and the 'WriteAddress' input ('" + writeAddressTypeDesc.ToString() + "') must be the same.");

		content.resize(depth * width, defaultValue);

		if (memoryBackdoor)
			*memoryBackdoor = this;

		for (int i = 1; i <= width; ++i) {

			rdDataOutput.emplace_back(this, defaultValue);
			wrDataInput.emplace_back(this, wrdatain(i));
		}
	}

	bus<T> get_output_bus()
	{
		bus<T> output;
		for (auto &driver : rdDataOutput)
			output.append(driver.GetNode());

		return output;
	}

	void writeMemoryBackdoor(int address, T const *data, int count) override
	{
		int size = (int)content.size();
		if ((address < 0) || ((address + count - 1) >= size))
			throw design_error(string_printf(GetFullName() + ": write address input (address = %d) is beyond the size of the memory (size = %d).", address, size));

		for (int i = 0; i < count; i++) {
			content[address + i] = *data;
			data++;
		}

		SetDirty();
	}

	void readMemoryBackdoor(int address, T *data, int count) override
	{
		int size = (int)content.size();
		if ((address < 0) || ((address + count - 1) >= size))
			throw design_error(string_printf(GetFullName() + ": write address input (address = %d) is beyond the size of the memory (size = %d).", address, size));

		for (int i = 0; i < count; i++) {
			*(data + i) = content[address + i];
		}
	}
};

} // namespace blocks
} // namespace backend

namespace blocks {

#define IMPLEMENT_MEMORY_FUNCTION(_type_)                                                                                                                                                                                 \
	node<_type_> InternalMemory(int size, node<dynfix> const &readAddress, node<bool> const &writeEnable, node<dynfix> const &writeAddress, node<_type_> const &writeData, IMemoryBackdoor<_type_> **memoryBackdoor)      \
	{                                                                                                                                                                                                                     \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::memory_block<_type_>>(size, readAddress, writeAddress, bus<_type_>(writeData), writeEnable, memoryBackdoor);                                         \
		return block.get_output_bus()[0];                                                                                                                                                                                 \
	}                                                                                                                                                                                                                     \
                                                                                                                                                                                                                          \
	bus<_type_> InternalMemory(int size, node<dynfix> const &readAddress, node<bool> const &writeEnable, node<dynfix> const &writeAddress, bus_access<_type_> const &writeData, IMemoryBackdoor<_type_> **memoryBackdoor) \
	{                                                                                                                                                                                                                     \
		auto &block = Design::GetCurrent().NewBlock<backend::blocks::memory_block<_type_>>(size, readAddress, writeAddress, writeData, writeEnable, memoryBackdoor);                                                      \
		return block.get_output_bus();                                                                                                                                                                                    \
	}

IMPLEMENT_MEMORY_FUNCTION(bool)
IMPLEMENT_MEMORY_FUNCTION(double)
IMPLEMENT_MEMORY_FUNCTION(std::int32_t)
IMPLEMENT_MEMORY_FUNCTION(std::int64_t)
IMPLEMENT_MEMORY_FUNCTION(dynfix)

} // namespace blocks
} // namespace dfx
