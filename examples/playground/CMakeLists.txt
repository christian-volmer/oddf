add_executable(playground
	main.cpp
)

target_link_libraries(playground PRIVATE oddf)
target_link_libraries(playground PRIVATE verilog)
