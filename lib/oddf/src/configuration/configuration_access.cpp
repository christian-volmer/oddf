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

	Classes and interfaces that support the creation of read and write
	registers and mapped memory access with automatic address generation
	and support for hierarchical blocks.

*/

#include "../global.h"
#include <cstring>

#include "configuration.h"

namespace dfx {
namespace configuration {

Access::Access(configuration::IController &theController, configuration::Namespace &theNamespace) :
	configController(theController),
	configNamespace(theNamespace)
{
}

void Access::ClearAll()
{
	configController.ClearAll();
}

configuration::IController &Access::GetController()
{
	return configController;
}


//
// InternalWrite functions
//

void Access::InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, std::int32_t const *values)
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::FixedPoint: {

			dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());
			bool wideRegister = typeDesc.GetWordWidth() > 32;

			for (int i = 0; i < count; ++i) {

				dynfix valueDynFix(values[i]);

				int align = typeDesc.GetFraction() - valueDynFix.GetFraction();

				if (align >= 0)
					valueDynFix.CopyShiftLeft(temp, align);
				else
					valueDynFix.CopyShiftRight(temp, -align);

				temp.OverflowWrapAround();

				std::uint32_t uTemp;

				if (wideRegister) {
					
					uTemp = static_cast<std::uint32_t>(temp.data[1]); configController.Write(startAddress + 2 * i + 0, &uTemp, 1);
					uTemp = static_cast<std::uint32_t>(temp.data[0]); configController.Write(startAddress + 2 * i + 1, &uTemp, 1);
				}
				else {

					uTemp = static_cast<std::uint32_t>(temp.data[0]); configController.Write(startAddress + i, &uTemp, 1);
				}
			}

			break;
		}


		default:
			throw std::bad_cast();
	}
}

void Access::InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, std::int64_t const *values)
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::FixedPoint: {

			dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());
			bool wideRegister = typeDesc.GetWordWidth() > 32;

			for (int i = 0; i < count; ++i) {

				dynfix valueDynFix(values[i]);

				int align = typeDesc.GetFraction() - valueDynFix.GetFraction();

				if (align >= 0)
					valueDynFix.CopyShiftLeft(temp, align);
				else
					valueDynFix.CopyShiftRight(temp, -align);

				temp.OverflowWrapAround();

				std::uint32_t uTemp;

				if (wideRegister) {

					uTemp = static_cast<std::uint32_t>(temp.data[1]); configController.Write(startAddress + 2 * i + 0, &uTemp, 1);
					uTemp = static_cast<std::uint32_t>(temp.data[0]); configController.Write(startAddress + 2 * i + 1, &uTemp, 1);
				}
				else {

					uTemp = static_cast<std::uint32_t>(temp.data[0]); configController.Write(startAddress + i, &uTemp, 1);
				}
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}

void Access::InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, double const *values)
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Double: {

			for (int i = 0; i < count; ++i) {

				std::uint64_t value64;

				static_assert(sizeof(double) == sizeof(std::int64_t), "double vs. int64 size mismatch.");
				std::memcpy(&value64, &values[i], sizeof(std::uint64_t));

				std::uint32_t values32[2] = { static_cast<std::uint32_t>(value64 >> 32), static_cast<std::uint32_t>(value64 & 0xffffffff) };
				configController.Write(startAddress + 2 * i + 0, values32, 2);
			}

			break;
		}

		case types::TypeDescription::FixedPoint: {

			dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());
			bool wideRegister = typeDesc.GetWordWidth() > 32;

			for (int i = 0; i < count; ++i) {

				dynfix valueDynFix(values[i]);
				
				int align = typeDesc.GetFraction() - valueDynFix.GetFraction();

				if (align >= 0)
					valueDynFix.CopyShiftLeft(temp, align);
				else
					valueDynFix.CopyShiftRight(temp, -align);

				temp.OverflowWrapAround();

				std::uint32_t uTemp;

				if (wideRegister) {

					uTemp = static_cast<std::uint32_t>(temp.data[1]); configController.Write(startAddress + 2 * i + 0, &uTemp, 1);
					uTemp = static_cast<std::uint32_t>(temp.data[0]); configController.Write(startAddress + 2 * i + 1, &uTemp, 1);
				}
				else {

					uTemp = static_cast<std::uint32_t>(temp.data[0]); configController.Write(startAddress + i, &uTemp, 1);
				}
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}

void Access::InternalWrite(int startAddress, int count, types::TypeDescription const &typeDesc, bool const *values)
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Boolean: {

			for (int i = 0; i < count; ++i) {

				std::uint32_t value32 = values[i] ? 1 : 0;
				configController.Write(startAddress + i, &value32, 1);
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}


//
// InternalReadFunctions
//

void Access::InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, std::int32_t *values) const
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::FixedPoint: {

			if (typeDesc.GetFraction() == 0 && ((typeDesc.IsSigned() && typeDesc.GetWordWidth() <= 32) || (!typeDesc.IsSigned() && typeDesc.GetWordWidth() <= 31))) {

				for (int i = 0; i < count; ++i) {

					dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());

					std::uint32_t uTemp;

					configController.Read(startAddress + i, &uTemp, 1);
					std::memcpy(temp.data, &uTemp, sizeof(uTemp));
					temp.OverflowWrapAround();

					values[i] = static_cast<std::int32_t>(static_cast<std::int64_t>(temp));
				}
			}
			else
				throw std::bad_cast();

			break;
		}

		default:
			throw std::bad_cast();
	}
}

void Access::InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, std::int64_t *values) const
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::FixedPoint: {

			if (typeDesc.GetFraction() == 0 && ((typeDesc.IsSigned() && typeDesc.GetWordWidth() <= 64) || (!typeDesc.IsSigned() && typeDesc.GetWordWidth() <= 63))) {

				bool wideRegister = typeDesc.GetWordWidth() > 32;

				for (int i = 0; i < count; ++i) {

					dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());

					std::uint32_t uTemp;

					if (wideRegister) {

						configController.Read(startAddress + 2 * i + 0, &uTemp, 1); std::memcpy(&temp.data[1], &uTemp, sizeof(uTemp));
						configController.Read(startAddress + 2 * i + 1, &uTemp, 1);	std::memcpy(&temp.data[0], &uTemp, sizeof(uTemp));
					}
					else {

						configController.Read(startAddress + i, &uTemp, 1); std::memcpy(&temp.data[0], &uTemp, sizeof(uTemp));
					}

					temp.OverflowWrapAround();

					values[i] = static_cast<std::int64_t>(temp);
				}
			}
			else
				throw std::bad_cast();

			break;
		}

		default:
			throw std::bad_cast();
	}
}

void Access::InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, double *values) const
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Double: {

			for (int i = 0; i < count; ++i) {

				std::uint32_t value32;
				configController.Read(startAddress + 2 * i + 0, &value32, 1);
				std::uint64_t value64 = (std::uint64_t)value32 << INT64_C(32);
				configController.Read(startAddress + 2 * i + 1, &value32, 1);

				value64 |= value32;

				static_assert(sizeof(double) == sizeof(std::int64_t), "double vs. int64 size mismatch.");
				std::memcpy(&values[i], &value64, sizeof(double));
			}

			break;
		}

		case types::TypeDescription::FixedPoint: {

			bool wideRegister = typeDesc.GetWordWidth() > 32;

			for (int i = 0; i < count; ++i) {

				dynfix temp(typeDesc.IsSigned(), typeDesc.GetWordWidth(), typeDesc.GetFraction());

				std::uint32_t uTemp;

				if (wideRegister) {

					configController.Read(startAddress + 2 * i + 0, &uTemp, 1); std::memcpy(&temp.data[1], &uTemp, sizeof(uTemp));
					configController.Read(startAddress + 2 * i + 1, &uTemp, 1);	std::memcpy(&temp.data[0], &uTemp, sizeof(uTemp));
				}
				else {

					configController.Read(startAddress + i, &uTemp, 1); std::memcpy(&temp.data[0], &uTemp, sizeof(uTemp));
				}

				temp.OverflowWrapAround();

				values[i] = static_cast<double>(temp);
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}

void Access::InternalRead(int startAddress, int count, types::TypeDescription const &typeDesc, bool *values) const
{
	switch (typeDesc.GetClass()) {

		case types::TypeDescription::Boolean: {

			for (int i = 0; i < count; ++i) {

				std::uint32_t value32;
				configController.Read(startAddress + i, &value32, 1);
				values[i] = value32 != 0;
			}

			break;
		}

		default:
			throw std::bad_cast();
	}
}

}
}
