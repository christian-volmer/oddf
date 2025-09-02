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

#include "ISimulatorAccess.h"

#include <oddf/simulator/backend/ISimulatorAccess.h>
#include <oddf/Iid.h>
#include <oddf/utility/GetInterfaceHelper.h>

#include "../../Expect.h"

#include <memory>

namespace oddf {

namespace testing::simulator::backend {

class ITestInterface1 : public virtual oddf::IObject {

public:

	virtual ~ITestInterface1() = default;

	virtual int TestFunction1() const = 0;
};

class ITestInterface2 : public virtual ITestInterface1 {

public:

	virtual ~ITestInterface2() = default;

	virtual int TestFunction2() const = 0;
};

class ITestInterface3 : public virtual oddf::IObject {

public:

	virtual ~ITestInterface3() = default;

	virtual int TestFunction3() const = 0;
};

} // namespace testing::simulator::backend

template<>
struct Iid<testing::simulator::backend::ITestInterface1> {

	static constexpr Uid value = { 0x5ece6339, 0x1045, 0x46ec, 0x8f, 0x68, 0x94, 0x2f, 0xe6, 0x55, 0xaf, 0x56 };
};

template<>
struct Iid<testing::simulator::backend::ITestInterface2> {

	static constexpr Uid value = { 0xabd6f4a2, 0xa611, 0x4f8f, 0xb6, 0xb8, 0xae, 0x2b, 0x5d, 0x8b, 0x84, 0x51 };
};

template<>
struct Iid<testing::simulator::backend::ITestInterface3> {

	static constexpr Uid value = { 0xacfbfe4, 0x41e2, 0x4058, 0xa7, 0x71, 0x33, 0x80, 0x5, 0xb6, 0xbf, 0x87 };
};

namespace testing::simulator::backend {

class TestSimulatorObject : public virtual ITestInterface1, public virtual ITestInterface2 {

public:

	virtual int TestFunction1() const override
	{
		return 1;
	}

	virtual int TestFunction2() const override
	{
		return 2;
	}

	virtual void *GetInterface(oddf::Uid const &iid) override
	{
		return oddf::utility::GetInterfaceHelper<
			ITestInterface1, ITestInterface2, IObject>::GetInterface(this, iid);
	}
};

class TestSimulatorAccess : public virtual oddf::simulator::backend::ISimulatorAccess {

private:

	std::unique_ptr<oddf::IObject> m_simulatorObject;

public:

	TestSimulatorAccess() :
		m_simulatorObject(new TestSimulatorObject())
	{
	}

	virtual void *GetNamedObjectInterface(std::string const &name, Uid const &iid) const override
	{
		if (name == "ExistingObject") {

			return m_simulatorObject->GetInterface(iid);
		}
		else
			throw Exception(ExceptionCode::NoResource);
	}

	using oddf::simulator::backend::ISimulatorAccess::GetNamedObjectInterface;

	virtual void RegisterClockable(oddf::simulator::backend::IClockable &) override
	{
		throw Exception(ExceptionCode::NotImplemented);
	}

	virtual void UnregisterClockable(oddf::simulator::backend::IClockable &) override
	{
		throw Exception(ExceptionCode::NotImplemented);
	}

	virtual oddf::IHierarchyNode const &GetNodeHierarchyRoot() const override
	{
		throw Exception(ExceptionCode::NotImplemented);
	}
};

void Test_ISimulatorAccess()
{
	TestSimulatorAccess simulatorAccess;

	auto &interface1 = simulatorAccess.GetNamedObjectInterface<ITestInterface1>("ExistingObject");

	Expect(interface1.TestFunction1() == 1);

	auto &obj = interface1.GetInterface<oddf::IObject>();
	auto &interface2 = obj.GetInterface<ITestInterface2>();
	Expect(interface2.TestFunction2() == 2);

	auto &interface2b = interface1.GetInterface<ITestInterface2>();
	Expect(interface2b.TestFunction2() == 2);

	ExpectThrows(ExceptionCode::NoResource, [&]() {
		simulatorAccess.GetNamedObjectInterface<ITestInterface1>("NonExistingObject");
	});

	ExpectThrows(ExceptionCode::NoInterface, [&]() {
		simulatorAccess.GetNamedObjectInterface<ITestInterface3>("ExistingObject");
	});
}

} // namespace testing::simulator::backend
} // namespace oddf
