cmake_minimum_required(VERSION 3.25)
project(lossless_audio_codec VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(LAC_BUILD_TESTS "Build the test suite" ON)
option(LAC_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(LAC_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

add_library(lac_warnings INTERFACE)
target_compile_options(lac_warnings INTERFACE
  -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow)

file(GLOB_RECURSE LAC_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
if(LAC_SOURCES)
  add_library(lac STATIC ${LAC_SOURCES})
  target_include_directories(lac PUBLIC "${PROJECT_SOURCE_DIR}/include")
  target_link_libraries(lac PRIVATE lac_warnings)
else()
  add_library(lac INTERFACE)
  target_include_directories(lac INTERFACE "${PROJECT_SOURCE_DIR}/include")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/cli/main.cpp")
  add_executable(lac_cli cli/main.cpp)
  set_target_properties(lac_cli PROPERTIES OUTPUT_NAME lac)
  target_link_libraries(lac_cli PRIVATE lac lac_warnings)
endif()

file(GLOB_RECURSE LAC_BENCH_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/bench/*.cpp")
if(LAC_BENCH_SOURCES)
  add_executable(lac_bench ${LAC_BENCH_SOURCES})
  target_link_libraries(lac_bench PRIVATE lac lac_warnings)
endif()

file(GLOB_RECURSE LAC_TEST_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/tests/*.cpp")
if(LAC_BUILD_TESTS AND LAC_TEST_SOURCES)
  include(FetchContent)
  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SYSTEM)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  include(GoogleTest)
  add_executable(lac_tests ${LAC_TEST_SOURCES})
  target_link_libraries(lac_tests PRIVATE lac lac_warnings GTest::gtest_main)
  gtest_discover_tests(lac_tests DISCOVERY_TIMEOUT 30)
endif()
