cmake_minimum_required(VERSION 3.20)
project(ReturnAddressSpoofer)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(ReturnAddressSpoofer INTERFACE)
target_include_directories(ReturnAddressSpoofer INTERFACE "Include")

add_executable(ReturnAddressSpooferExample "Example/Main.cpp")
target_compile_options(ReturnAddressSpooferExample PUBLIC "-O2" "-ffunction-sections")
target_link_libraries(ReturnAddressSpooferExample PRIVATE ReturnAddressSpoofer)

enable_testing()
add_test(NAME ReturnAddressSpooferExample COMMAND $<TARGET_FILE:ReturnAddressSpooferExample>)

add_subdirectory("ObjectFileRewriter")
add_custom_command(
    TARGET ReturnAddressSpooferExample PRE_LINK
	COMMAND "$<TARGET_FILE:ObjectFileRewriter>" ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS ObjectFileRewriter
)
