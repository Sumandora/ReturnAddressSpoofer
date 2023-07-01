cmake_minimum_required(VERSION 3.20)
project(ReturnAddressSpoofer)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer -O2")

add_executable(ReturnAddressSpoofer Source/Main.cpp)
target_include_directories(ReturnAddressSpoofer PRIVATE include)
