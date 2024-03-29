add_library(oddf
	src/block_base.cpp
	src/debug.cpp
	src/design.cpp
	src/formatting.cpp
	src/hierarchy.cpp
	src/messages.cpp
	src/simulator.cpp
	src/types.cpp
	src/blocks/bit_compose.cpp
	src/blocks/bit_compose_dynfix.cpp
	src/blocks/bit_extract.cpp
	src/blocks/bit_extract_dynfix.cpp
	src/blocks/constant.cpp
	src/blocks/convergent_cast.cpp
	src/blocks/decide.cpp
	src/blocks/decide_dynfix.cpp
	src/blocks/delay.cpp
	src/blocks/floor_cast.cpp
	src/blocks/label.cpp
	src/blocks/memory.cpp
	src/blocks/modulo.cpp
	src/blocks/modulo_dynfix.cpp
	src/blocks/nearest_cast.cpp
	src/blocks/operators_flat.cpp
	src/blocks/operators_flat_dynfix_plus.cpp
	src/blocks/operators_flat_dynfix_times.cpp
	src/blocks/operators_relational.cpp
	src/blocks/operators_relational_dynfix.cpp
	src/blocks/operators_unary.cpp
	src/blocks/operators_unary_dynfix.cpp
	src/blocks/power_of_two.cpp
	src/blocks/probe.cpp
	src/blocks/random.cpp
	src/blocks/reinterpret_cast.cpp
	src/blocks/replace.cpp
	src/blocks/select.cpp
	src/blocks/select_dynfix.cpp
	src/blocks/signal.cpp
	src/blocks/spare.cpp
	src/blocks/terminate.cpp
	src/configuration/configuration_access.cpp
	src/configuration/configuration_namespace.cpp
	src/generator/generator.cpp
	src/generator/generator_instances.cpp
	src/generator/generator_port_naming.cpp
	src/generator/generator_port_placing.cpp
	src/generator/properties.cpp
	src/helpers/h_bit_extract.cpp
	src/modules/logger.cpp
	src/modules/recorder.cpp
	src/modules/register_file.cpp
	src/modules/serial_configuration_builder.cpp
	src/modules/serial_configuration_controller.cpp
	src/modules/sink.cpp
	src/modules/source.cpp
	src/modules/throttled_source.cpp
)

target_include_directories(oddf PUBLIC include)

target_compile_features(oddf PUBLIC cxx_std_17)

target_precompile_headers(oddf PRIVATE src/global.h)
