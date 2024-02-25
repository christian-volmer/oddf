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

	Label blocks are used internally by the design framework to assign
	names to parts of the design.

*/

#pragma once

namespace dfx {
namespace blocks {

#define DECLARE_LABEL_FUNCTION(_type_) \
	void _Label(node<_type_> const &input, std::string const &labelClass, std::string const &label); \
	void _Label(bus_access<_type_> const &inputBus, std::string const &labelClass, std::string const &label);

DECLARE_LABEL_FUNCTION(bool)
DECLARE_LABEL_FUNCTION(double)
DECLARE_LABEL_FUNCTION(std::int32_t)
DECLARE_LABEL_FUNCTION(std::int64_t)
DECLARE_LABEL_FUNCTION(dynfix)

#undef DECLARE_LABEL_FUNCTION

}
}

std::string inline _Label_AutomaticName(std::string id)
{
	std::transform(id.begin(), id.end(), id.begin(), [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ? c : '_'; });
	return id;
}

#define DFX_INPUT_NAME(input, name) dfx::blocks::_Label((input), "input", (name))
#define DFX_INPUT(input) dfx::blocks::_Label((input), "input", _Label_AutomaticName(#input))

#define DFX_UNUSED_INPUT(input) \
	{ \
		dfx::blocks::_Label((input), "input", _Label_AutomaticName(#input)); \
		dfx::ScopedGotoHierarchyLevel _localHierarchy(dfx::Design::GetCurrent().GetHierarchy().GetCurrentLevel()->GetParent()); \
		dfx::blocks::Zero((input)); \
	}

#define DFX_UNUSED_INPUT_NAME(input, name) \
	{ \
		dfx::blocks::_Label((input), "input", (name)); \
		dfx::ScopedGotoHierarchyLevel _localHierarchy(dfx::Design::GetCurrent().GetHierarchy().GetCurrentLevel()->GetParent()); \
		dfx::blocks::Zero((input)); \
	}

#define DFX_OUTPUT_NAME(output, name) dfx::blocks::_Label((output), "output", (name))
#define DFX_OUTPUT(output) dfx::blocks::_Label((output), "output", _Label_AutomaticName(#output))

#define DFX_UNUSED_OUTPUT(output) \
	{ \
		dfx::blocks::_Label((output), "output", _Label_AutomaticName(#output)); \
		dfx::ScopedGotoHierarchyLevel _localHierarchy(dfx::Design::GetCurrent().GetHierarchy().GetCurrentLevel()->GetParent()); \
		dfx::blocks::Terminate((output)); \
	}

#define DFX_UNUSED_OUTPUT_NAME(output, name) \
	{ \
		dfx::blocks::_Label((output), "output", (name)); \
		dfx::ScopedGotoHierarchyLevel _localHierarchy(dfx::Design::GetCurrent().GetHierarchy().GetCurrentLevel()->GetParent()); \
		dfx::blocks::Terminate((output)); \
	}
