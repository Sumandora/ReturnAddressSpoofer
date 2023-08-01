cmake_minimum_required(VERSION 3.20)
project(ReturnAddressSpoofer)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(ReturnAddressSpoofer "Example/Main.cpp")
target_compile_options(ReturnAddressSpoofer PUBLIC "-O2" "-ffunction-sections")

target_include_directories(ReturnAddressSpoofer PUBLIC "Include")
add_subdirectory("ObjectFileRewriter")
add_custom_command(
	TARGET ${CMAKE_PROJECT_NAME} PRE_LINK
	COMMAND "$<TARGET_FILE:ObjectFileRewriter>" "${CMAKE_BINARY_DIR}/CMakeFiles/${CMAKE_PROJECT_NAME}.dir"
	DEPENDS ObjectFileRewriter
)
