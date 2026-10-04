cmake_minimum_required(VERSION 3.25)
project(lossless_audio_codec VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(lac_warnings INTERFACE)
target_compile_options(lac_warnings INTERFACE
  -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow)

add_library(lac
  src/bit_writer.cpp)
target_include_directories(lac PUBLIC include)
target_link_libraries(lac PRIVATE lac_warnings)

include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)
add_executable(lac_tests
  tests/bit_writer_test.cpp)
target_link_libraries(lac_tests PRIVATE lac lac_warnings GTest::gtest_main)
gtest_discover_tests(lac_tests DISCOVERY_TIMEOUT 30)
