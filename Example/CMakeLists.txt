add_library(TargetLibrary SHARED "${PROJECT_SOURCE_DIR}/Example/Target/Main.cpp")

add_executable(ReturnAddressSpooferExample "${PROJECT_SOURCE_DIR}/Example/Attacker/Main.cpp")

target_link_libraries(ReturnAddressSpooferExample PRIVATE TargetLibrary ReturnAddressSpoofer)
target_compile_features(ReturnAddressSpooferExample PUBLIC cxx_std_20)
