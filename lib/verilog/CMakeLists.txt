add_library(verilog
	verilog.cpp
	entities/add.cpp
	entities/bit_compose.cpp
	entities/bit_extract.cpp
	entities/checker.cpp
	entities/compare.cpp
	entities/constant.cpp
	entities/decide.cpp
	entities/default.cpp
	entities/delay.cpp
	entities/floor_cast.cpp
	entities/ignore.cpp
	entities/input_port.cpp
	entities/instance.cpp
	entities/logic.cpp
	entities/memory_dp.cpp
	entities/mul.cpp
	entities/negate.cpp
	entities/not.cpp
	entities/output_port.cpp
	entities/recorder.cpp
	entities/reinterpret_cast.cpp
	entities/select.cpp
	entities/spare.cpp
	entities/stimulus.cpp
)

# target_include_directories(verilog PUBLIC include)

target_compile_features(verilog PUBLIC cxx_std_17)
target_precompile_headers(verilog PRIVATE global.h)
