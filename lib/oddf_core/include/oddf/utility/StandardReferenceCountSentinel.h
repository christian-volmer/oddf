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

    Provides a standard implementation of the `IReferenceCountSentinel`
    interface.

*/

#pragma once

#include "IReferenceCountSentinel.h"

namespace oddf::utility {

/*
    Standard implementation of the `IReferenceCountSentinel` that throws
    on desctruction onless the reference count has reached zero.
*/
class StandardReferenceCountSentinel : public virtual IReferenceCountSentinel {

private:

	size_t m_ReferenceCount;

public:

	StandardReferenceCountSentinel();

	~StandardReferenceCountSentinel() override;

	StandardReferenceCountSentinel(StandardReferenceCountSentinel const &) = delete;
	StandardReferenceCountSentinel(StandardReferenceCountSentinel &&) = delete;

	void operator=(StandardReferenceCountSentinel const &) = delete;
	void operator=(StandardReferenceCountSentinel &&) = delete;

	virtual void Increment() noexcept override;
	virtual void Decrement() override;
	virtual size_t GetValue() const noexcept override;
};

} // namespace oddf::utility
