add_executable(test
	main.cpp
)

target_link_libraries(test PRIVATE oddf)
target_link_libraries(test PRIVATE verilog)
