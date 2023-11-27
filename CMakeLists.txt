cmake_minimum_required(VERSION 3.20)

include_guard()

project(ReturnAddressSpoofer)

add_library(ReturnAddressSpoofer INTERFACE)
target_include_directories(ReturnAddressSpoofer INTERFACE "${PROJECT_SOURCE_DIR}/Include")

if(PROJECT_IS_TOP_LEVEL)
	add_subdirectory("ObjectFileRewriter")

	add_subdirectory("Example")

	enable_testing()
	add_test(NAME ReturnAddressSpooferExample COMMAND $<TARGET_FILE:ReturnAddressSpooferExample>)
endif()
