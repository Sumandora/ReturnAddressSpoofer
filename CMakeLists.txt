cmake_minimum_required(VERSION 3.20)

include_guard()

project(ReturnAddressSpoofer)

add_library(ReturnAddressSpoofer INTERFACE)
target_include_directories(ReturnAddressSpoofer INTERFACE "${PROJECT_SOURCE_DIR}/Include")

if(PROJECT_IS_TOP_LEVEL)
	add_executable(ReturnAddressSpooferExample "${PROJECT_SOURCE_DIR}/Example/Main.cpp")
	target_compile_options(ReturnAddressSpooferExample PUBLIC "-Ofast" "-ffunction-sections")
	target_link_libraries(ReturnAddressSpooferExample PRIVATE ReturnAddressSpoofer)
	target_compile_features(ReturnAddressSpooferExample PUBLIC cxx_std_20)

	enable_testing()
	add_test(NAME ReturnAddressSpooferExample COMMAND $<TARGET_FILE:ReturnAddressSpooferExample>)

	add_subdirectory("ObjectFileRewriter")
	add_custom_command(
		TARGET ReturnAddressSpooferExample PRE_LINK
		COMMAND "$<TARGET_FILE:ObjectFileRewriter>" "${CMAKE_CURRENT_BINARY_DIR}"
		DEPENDS ObjectFileRewriter
	)
endif()
