cmake_minimum_required(VERSION 3.20)
project(ObjectFileRewriter)

set(CMAKE_CXX_STANDARD 20)

add_executable(ObjectFileRewriter Source/Main.cpp)

target_compile_options(ObjectFileRewriter PRIVATE "-O3")
target_include_directories(ObjectFileRewriter PRIVATE "Dependencies/distorm/include")
file(GLOB_RECURSE DISTORM_SOURCE_FILES "Dependencies/distorm/src/*.c")
target_sources(ObjectFileRewriter PRIVATE ${DISTORM_SOURCE_FILES})
