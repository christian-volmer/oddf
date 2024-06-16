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

    Classes that make up a design: node, forward_node, InputPin,
    OutputPin

*/

#pragma once

#include "global.h"

#include "block_base.h"
#include "messages.h"
#include "types.h"

#include <oddf/design/backend/IDesign.h>
#include <oddf/design/backend/IBlockInput.h>
#include <oddf/design/backend/IBlockOutput.h>

#if defined(_MSC_VER) && _MSC_VER < 1900
#define NOEXCEPT
#define ALIGN __declspec(align(8))
#pragma warning(disable : 4324) // warning about alignment
#else
#define NOEXCEPT noexcept
#define ALIGN
#endif

namespace dfx {

class Design;
template<typename T>
class forward_bus;

namespace backend {

template<typename T>
class InputPin;
template<typename T>
class OutputPin;
template<typename T>
class temporary_block;
template<typename T>
class identity_block;

} // namespace backend

//
// node<T> definition
//

template<typename T>
class node {

protected:

	using internalType = typename types::TypeTraits<T>::internalType;

	backend::OutputPin<internalType> *driver;

	explicit node(backend::OutputPin<internalType> *OutputPin);

	bool IsDriven() const;

	void operator<<=(node<T> const &rhs);

	friend class Design;
	friend backend::OutputPin<internalType>;
	friend backend::InputPin<internalType>;
	template<typename>
	friend class forward_bus;

public:

	node();
	node(node<T> const &other);

	// TODO: According to https://stackoverflow.com/questions/8001823/how-to-enforce-move-semantics-when-a-vector-grows
	// the move constructor should be noexcept in order to avoid copying when an std::vector resizes. (Our copy constructor
	// is non-trivial with significant side-effects.). Visual Studio 2013 does not support 'noexcept', but std::vector moves
	// anyway.
	node(node<T> &&other) NOEXCEPT;

	node<T> &operator=(node<T> const &other);

	backend::OutputPin<internalType> *GetDriver() const
	{
		return driver;
	}

	types::TypeDescription GetType() const
	{
		return GetDriver()->GetType();
	}
};

//
// forware_node<T> definition
//

template<typename T>
class forward_node : public node<typename types::TypeTraits<T>::internalType> {

	// TODO: forward_node is a hack, which is OK for the time being. Need to streamline node-interface. What happens to nodes passed as non-const reference
	// to function? Function is allowed to call = or <<=, but which is not obvious from the function interface.
	// Something along those lines (or completely different)
	// E.g. remove operator <<= from node, only have it in forward_node
	// Make node base-class of forward_node private.
	// Add cast-to-node const &operator to forward_node (so forward_node behaves like node on right-hand sides)
	// Have function drive_into_node drive_into(forward_node &), drives back into forward_node when drive_into_node is assigned via =.

	using internalType = typename types::TypeTraits<T>::internalType;

public:

	forward_node();
	forward_node(internalType const &templateType);
	forward_node(forward_node<T> const &other) = delete;
	forward_node(forward_node<T> &&other) NOEXCEPT;

	// TODO: do we allow moving if the left hand side has no consumer? Helps to support reg_Something = regFile.AddReadRegister<...>(...); with reg_Something being a class field.
	void operator=(forward_node<T> &&other) = delete;

	using node<internalType>::operator<<=;

	friend void operator>>=(node<internalType> const &other, forward_node<T> &self)
	{
		self <<= other;
	}

	// Hide the = operator to avoid calling it accidentally.
	node<internalType> &operator=(node<internalType> const &other) = delete;
};

//
// design definition
//

class Design : public oddf::design::backend::IDesign {

private:

	std::unique_ptr<class Hierarchy> hierarchy;
	std::vector<std::unique_ptr<backend::BlockBase>> blocks;

	friend class Simulator;

public:

	Design();

	bool hasCustomDefaultEnable;
	node<bool> customDefaultEnable;

	template<typename blockT, typename... argTs>
	blockT &NewBlock(argTs &&...args);

	class Hierarchy &GetHierarchy() const;

	static Design &GetCurrent();

	std::vector<std::unique_ptr<backend::BlockBase>> const &GetBlocks() const;

	static bool HasCustomDefaultEnable()
	{
		return GetCurrent().hasCustomDefaultEnable;
	}

	void Report(std::basic_ostream<char> &os) const;

	//
	// IDesign implementation
	//

	virtual oddf::utility::CollectionView<oddf::design::backend::IDesignBlock> GetBlockCollection() const override;
};

namespace backend {

//
// backend::OutputPin<T> definition
//

class OutputPinBase : public oddf::design::backend::IBlockOutput {

protected:

	int index;
	BlockBase *owner;

public:

	OutputPinBase(BlockBase *owner) :
		owner(owner)
	{
		index = (int)owner->outputPins.size();
		owner->outputPins.push_back(this);
	}

	virtual ~OutputPinBase()
	{
	}

	virtual bool IsConnected() const = 0;
	virtual types::TypeDescription GetType() const = 0;

	std::string GetName() const
	{
		int groupIndex = 0, busSize = 0, busIndex = 0;
		return owner->GetOutputPinDescription(index, groupIndex, busSize, busIndex);
	}

	std::string GetNameAndInfo(int &groupIndex, int &busSize, int &busIndex) const
	{
		return owner->GetOutputPinDescription(index, groupIndex, busSize, busIndex);
	}

	BlockBase const *GetOwner() const
	{
		return owner;
	}

	virtual oddf::design::backend::IDesignBlock const &GetOwningBlock() const override
	{
		assert(GetOwner());
		return *GetOwner();
	}

	virtual size_t GetIndex() const override
	{
		assert(GetOwner());
		return index;
	}
};

template<typename T>
class OutputPin : public OutputPinBase {

private:

	std::list<InputPin<T> *> drivenPins;

	void RegisterPin(InputPin<T> *pin);
	void UnregisterPin(InputPin<T> *pin);

	friend class InputPin<T>;
	friend class node<T>;
	friend class temporary_block<T>;
	friend class identity_block<T>;

public:

	ALIGN T value;

	OutputPin(BlockBase *owner, T const &init);
	virtual ~OutputPin();

	node<T> GetNode();

	bool IsConnected() const override;
	types::TypeDescription GetType() const override;

	OutputPin(OutputPin<T> const &) = delete;
	OutputPin(OutputPin<T> &&) = delete;
	void operator=(OutputPin<T> const &) = delete;
	void operator=(OutputPin<T> &&) = delete;
};

//
// backend::InputPin<T> definition
//

class InputPinBase : public oddf::design::backend::IBlockInput {

protected:

	int index;
	BlockBase *owner;

public:

	InputPinBase(BlockBase *owner) :
		owner(owner)
	{
		index = (int)owner->inputPins.size();
		owner->inputPins.push_back(this);
	}

	virtual ~InputPinBase()
	{
	}

	std::string GetName() const
	{
		return owner->GetInputPinName(index);
	}

	BlockBase const *GetOwner() const
	{
		return owner;
	}

	virtual OutputPinBase const *GetDrivingPin() const = 0;
};

template<typename T>
class InputPin : public InputPinBase {

private:

	OutputPin<T> *driver;

	void Disconnect();

	friend class OutputPin<T>;
	friend class identity_block<T>;
	friend class BlockBase;

public:

	InputPin(BlockBase *owner, node<T> const &other);
	virtual ~InputPin();

	void Connect(node<T> const &other);

	T const &GetValue() const
	{
		return driver->value;
	}

	BlockBase *GetDrivingBlock() const
	{
		if (driver)
			return driver->owner;
		else
			return nullptr;
	}

	virtual oddf::design::backend::IBlockOutput &GetDriver() const override
	{
		assert(driver);
		return *driver;
	}

	virtual bool IsConnected() const override
	{
		return GetDrivingBlock() != nullptr;
	}

	OutputPinBase const *GetDrivingPin() const override
	{
		return driver;
	}

	InputPin(InputPin<T> const &) = delete;
	InputPin(InputPin<T> &&) = delete;
	void operator=(InputPin<T> const &) = delete;
	void operator=(InputPin<T> &&) = delete;
};

//
// backend::temporary_block<T> definition
//

template<typename T>
class temporary_block : public BlockBase {

private:

	source_blocks_t GetSourceBlocks() const override;
	bool IsTemporary() const override;

	bool CanEvaluate() const override;
	void Evaluate() override;

	template<typename S>
	friend class node;

public:

	OutputPin<T> Output;
	temporary_block(T const &init);
};

//
// backend::identity_block<T> definition
//

template<typename T>
class identity_block : public BlockBase {

private:

	InputPin<T> Input;
	OutputPin<T> Output;

	source_blocks_t GetSourceBlocks() const override;
	void Simplify() override;

	bool CanEvaluate() const override;
	void Evaluate() override;

	friend class node<T>;

public:

	identity_block(node<T> const &other);
};

} // namespace backend

////////////////////////////////////////////////////////////////////////////////
// implementation
////////////////////////////////////////////////////////////////////////////////

//
// node<T> implementation
//

template<typename T>
inline node<T>::node() :
	driver(&Design::GetCurrent().NewBlock<backend::temporary_block<internalType>>(T()).Output)
{
}

template<typename T>
inline node<T>::node(node<T> const &other) :
	driver(other.IsDriven() ? other.driver : &Design::GetCurrent().NewBlock<backend::identity_block<internalType>>(other).Output)
{
}

template<typename T>
inline node<T>::node(node<T> &&other) NOEXCEPT : driver(std::move(other.driver))
{
}

template<typename T>
inline node<T>::node(backend::OutputPin<internalType> *outputPin) :
	driver(outputPin)
{
}

template<typename T>
inline node<T> &node<T>::operator=(node<T> const &rhs)
{
	// same as copy constructor
	if (rhs.GetDriver() != nullptr)
		driver = rhs.IsDriven() ? rhs.driver : &Design::GetCurrent().NewBlock<backend::identity_block<T>>(rhs).Output;
	else
		driver = nullptr;

	return *this;
}

template<typename T>
inline bool node<T>::IsDriven() const
{
	// A node is refered to as 'driven' if its OutputPin is not a temporary one.
	return !driver->owner->IsTemporary();
}

template<typename T>
inline void node<T>::operator<<=(node<T> const &rhs)
{
	if (IsDriven())
		// We already have a OutputPin and the <<= operator cannot be used to change this.
		// Example: node<double> n = constant(1.0); n <<= otherNode; // throws this exception
		throw design_error("Operator <<= : this operator can only be used if the left-hand side node has no driver.");

	if (!types::IsCompatible(driver->value, rhs.GetDriver()->value))
		throw design_error("Operator <<= : requires both side to have the same type. Here you are trying to do '" + types::GetDescription(driver->value).ToString() + "' <<= '" + types::GetDescription(rhs.GetDriver()->value).ToString() + "'.");

	// We take all pin driven by this node an connect them to the OutputPin of 'rhs'.
	while (!driver->drivenPins.empty())
		(*driver->drivenPins.begin())->Connect(rhs);

	// We now make the OutputPin of 'rhs' our own.
	driver = rhs.IsDriven() ? rhs.driver : &Design::GetCurrent().NewBlock<backend::identity_block<internalType>>(rhs).Output;
}

//
// forward_node<T> implementation
//

template<typename T>
inline forward_node<T>::forward_node() :
	node<internalType>(&Design::GetCurrent().NewBlock<backend::temporary_block<internalType>>(T()).Output)
{
	if (!types::IsInitialised(this->driver->value))
		throw design_error("The given data type cannot be used with forward_node.");
}

template<typename T>
inline forward_node<T>::forward_node(internalType const &templateType) :
	node<internalType>(&Design::GetCurrent().NewBlock<backend::temporary_block<internalType>>(templateType).Output)
{
	if (!types::IsInitialised(this->driver->value))
		throw design_error("The given data type cannot be used with forward_node.");
}

template<typename T>
inline forward_node<T>::forward_node(forward_node<T> &&other) NOEXCEPT : node<internalType>(std::move(other))
{
}

//
// Design::new_block implementation
//

template<typename blockT, typename... argTs>
inline blockT &Design::NewBlock(argTs &&...args)
{
	blockT *block = new blockT(std::forward<argTs>(args)...);
	blocks.push_back(std::unique_ptr<blockT>(block));
	return *block;
}

namespace backend {

//
// backend::OutputPin<T> implementation
//

template<typename T>
inline OutputPin<T>::OutputPin(BlockBase *owner, T const &init) :
	OutputPinBase(owner),
	drivenPins(),
	value(init)
{
}

template<typename T>
inline OutputPin<T>::~OutputPin()
{
	while (!drivenPins.empty())
		(*drivenPins.begin())->Disconnect();
}

template<typename T>
inline void OutputPin<T>::RegisterPin(InputPin<T> *pin)
{
	assert(pin != nullptr);
	drivenPins.push_back(pin);
}

template<typename T>
inline void OutputPin<T>::UnregisterPin(InputPin<T> *pin)
{
	drivenPins.erase(std::find(std::begin(drivenPins), std::end(drivenPins), pin));
}

template<typename T>
inline node<T> OutputPin<T>::GetNode()
{
	return node<T>(this);
}

template<typename T>
inline bool OutputPin<T>::IsConnected() const
{
	return !drivenPins.empty();
}

template<typename T>
inline types::TypeDescription OutputPin<T>::GetType() const
{
	return types::GetDescription(value);
}

//
// backend::InputPin<T> implementation
//

template<typename T>
inline InputPin<T>::InputPin(BlockBase *owner, node<T> const &other) :
	InputPinBase(owner),
	driver(nullptr)
{
	Connect(other);
}

template<typename T>
inline InputPin<T>::~InputPin()
{
	Disconnect();
}

template<typename T>
inline void InputPin<T>::Disconnect()
{
	if (driver) {

		driver->UnregisterPin(this);
		driver = nullptr;
	}
}

template<typename T>
inline void InputPin<T>::Connect(node<T> const &Other)
{
	if (!Other.driver)
		throw design_error("Can only connect to nodes that have a driver");

	Disconnect();

	driver = Other.driver;

	if (driver)
		driver->RegisterPin(this);
}

//
// backend::temporary_block<T> implementation
//

template<typename T>
inline temporary_block<T>::temporary_block(T const &init) :
	BlockBase("temporary"),
	Output(this, init)
{
}

template<typename T>
inline bool temporary_block<T>::CanEvaluate() const
{
	if (Output.IsConnected()) {

		std::unordered_set<BlockBase const *> blocks;
		for (auto pin : Output.drivenPins)
			blocks.insert(pin->GetOwner());

		for (BlockBase const *block : blocks)
			design_info("Affected block: '" + block->GetFullName() + "'.");

		throw design_error("Blocks have undriven inputs. Did you forget to assign a driver to a node using operator <<= ?");
	}

	return false;
}

template<typename T>
inline void temporary_block<T>::Evaluate()
{
}

template<typename T>
inline BlockBase::source_blocks_t temporary_block<T>::GetSourceBlocks() const
{
	return source_blocks_t();
}

template<typename T>
bool temporary_block<T>::IsTemporary() const
{
	return true;
}

//
// backend::identity_block<T> implementation
//

template<typename T>
inline identity_block<T>::identity_block(node<T> const &other) :
	BlockBase("identity"),
	Input(this, other),
	Output(this, Input.driver->value)
{
}

template<typename T>
inline bool identity_block<T>::CanEvaluate() const
{
	return Output.IsConnected();
}

template<typename T>
inline void identity_block<T>::Evaluate()
{
	Output.value = Input.GetValue();
}

template<typename T>
inline BlockBase::source_blocks_t identity_block<T>::GetSourceBlocks() const
{
	if (Input.GetDrivingBlock() != nullptr) {

		design_info(GetFullName() + ": identity block was not removed during execution graph optimisation.");
		return source_blocks_t({ Input.GetDrivingBlock() });
	}
	else
		return source_blocks_t();
}

template<typename T>
inline void identity_block<T>::Simplify()
{
	// Only remove this block if it is not driving itself (computational cycle)
	if (Input.GetDrivingBlock() != this) {

		while (!Output.drivenPins.empty()) {

			auto *pin = *Output.drivenPins.begin();
			pin->Disconnect();
			Input.driver->RegisterPin(pin);
			pin->driver = Input.driver;
		}

		Input.Disconnect();
	}
}

} // namespace backend

} // namespace dfx

#undef NOEXCEPT
