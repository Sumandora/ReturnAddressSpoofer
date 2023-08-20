cmake_minimum_required(VERSION 3.20)
project(ObjectFileRewriter)

add_executable(ObjectFileRewriter Source/Main.cpp)

target_compile_options(ObjectFileRewriter PRIVATE "-Ofast")
target_compile_features(ObjectFileRewriter PUBLIC cxx_std_98)

include(FetchContent)

FetchContent_Declare(distorm
	GIT_REPOSITORY https://github.com/gdabah/distorm.git
	GIT_PROGRESS TRUE
	GIT_SHALLOW TRUE
	GIT_TAG 3.5.2b)
FetchContent_MakeAvailable(distorm)
file(GLOB_RECURSE distorm_SRC_FILES ${distorm_SOURCE_DIR}/src/*.c)
add_library(distorm STATIC ${distorm_SRC_FILES})
target_include_directories(distorm PUBLIC ${distorm_SOURCE_DIR}/include)

target_link_libraries(ObjectFileRewriter distorm)
