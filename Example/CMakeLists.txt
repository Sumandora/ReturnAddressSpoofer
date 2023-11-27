
add_library(TargetLibrary SHARED "${PROJECT_SOURCE_DIR}/Example/Target/Main.cpp")

add_executable(ReturnAddressSpooferExample "${PROJECT_SOURCE_DIR}/Example/Attacker/Main.cpp")
target_compile_options(ReturnAddressSpooferExample PUBLIC "-ffunction-sections")
target_link_libraries(ReturnAddressSpooferExample PRIVATE TargetLibrary ReturnAddressSpoofer)
target_compile_features(ReturnAddressSpooferExample PUBLIC cxx_std_20)

add_custom_command(
        TARGET ReturnAddressSpooferExample PRE_LINK
        COMMAND "$<TARGET_FILE:ObjectFileRewriter>" "-i" "${CMAKE_BINARY_DIR}/Example/CMakeFiles/ReturnAddressSpooferExample.dir"
        DEPENDS ObjectFileRewriter
)