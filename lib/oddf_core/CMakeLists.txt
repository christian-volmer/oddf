add_library(oddf_core
	src/oddf/ResourcePath.cpp
	src/oddf/Uid.cpp
	src/oddf/design/NodeType.cpp
	src/oddf/utility/CopyBoolean.cpp
	src/oddf/utility/CopyInteger.cpp
	src/oddf/utility/StandardReferenceCountSentinel.cpp
)

target_include_directories(oddf_core PUBLIC include)

target_compile_features(oddf_core PUBLIC cxx_std_17)

# target_precompile_headers(oddf_core PRIVATE src/global.h)
