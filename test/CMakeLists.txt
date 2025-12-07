cmake_minimum_required(VERSION 3.20)
project(icu_test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(".." icu-portable)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG        "v1.17.0"
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(icu_gtest test.cc)

# Ensure ICU libraries are output to the same directory as the test executable
get_target_property(EXE_DIR icu_gtest RUNTIME_OUTPUT_DIRECTORY)
if(NOT EXE_DIR)
  set(EXE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()

set_target_properties(icuuc icui18n icuio PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${EXE_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${EXE_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${EXE_DIR}"
)

target_link_libraries(icu_gtest PRIVATE ICU::uc ICU::i18n ICU::io GTest::gtest_main)

add_test(NAME icu_gtest COMMAND icu_gtest)
