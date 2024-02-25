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

	Includes frequently used header files.

*/

#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <condition_variable>
#include <cstdarg>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iomanip>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "messages.h"

#include "types.h"
#include "node.h"
#include "bus.h"
#include "inout.h"

#include "simulator.h"
#include "hierarchy.h"

#include "debug.h"

#include "blocks/bit_compose.h"
#include "blocks/bit_extract.h"
#include "blocks/constant.h"
#include "blocks/decide.h"
#include "blocks/delay.h"
#include "blocks/floor_cast.h"
#include "blocks/function.h"
#include "blocks/label.h"
#include "blocks/memory.h"
#include "blocks/modulo.h"
#include "blocks/operators_unary.h"
#include "blocks/operators_flat.h"
#include "blocks/operators_relational.h"
#include "blocks/power_of_two.h"
#include "blocks/probe.h"
#include "blocks/random.h"
#include "blocks/reinterpret_cast.h"
#include "blocks/replace.h"
#include "blocks/select.h"
#include "blocks/signal.h"
#include "blocks/terminate.h"
