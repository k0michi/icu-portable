cmake_minimum_required(VERSION 3.20)
project(icu_test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(".." icu-portable)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(icu_gtest test.cc)

target_link_libraries(icu_gtest PRIVATE ICU::uc ICU::i18n ICU::io GTest::gtest_main)

add_test(NAME icu_gtest COMMAND icu_gtest)
