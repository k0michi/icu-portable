cmake_minimum_required(VERSION 3.27)
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

target_link_libraries(icu_gtest PRIVATE ICU::uc ICU::i18n ICU::io GTest::gtest_main)

add_test(NAME icu_gtest COMMAND icu_gtest)

# Ensure ICU libraries are output to the same directory as the test executable (for Windows)
add_custom_command(TARGET icu_gtest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_RUNTIME_DLLS:icu_gtest>
    $<TARGET_FILE_DIR:icu_gtest>
  COMMAND_EXPAND_LISTS
)