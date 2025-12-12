cmake_minimum_required(VERSION 3.20)

project(icu-portable LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ICU_VER_MAJOR "78")
set(ICU_VER_MINOR "1")
set(ICU_VER "${ICU_VER_MAJOR}.${ICU_VER_MINOR}")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

find_package(Python REQUIRED COMPONENTS Interpreter)

# 1. Fetch ICU4C source code
include(FetchContent)

message(STATUS "Fetching ICU4C Source...")
FetchContent_Declare(
    icu_src
    URL "https://github.com/unicode-org/icu/releases/download/release-${ICU_VER}/icu4c-${ICU_VER}-sources.tgz"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(icu_src)
set(ICU_SOURCE_DIR "${icu_src_SOURCE_DIR}/source")

# 2. Endianness detection and fetch ICU data binary
if(CMAKE_C_BYTE_ORDER STREQUAL "BIG_ENDIAN")
    set(ICU_ENDIAN_CHAR "b")
    message(STATUS "System is Big Endian. Using 'b' data.")
else()
    set(ICU_ENDIAN_CHAR "l")
    message(STATUS "System is Little Endian. Using 'l' data.")
endif()

message(STATUS "Fetching ICU4C Prebuilt Data...")
FetchContent_Declare(
    icu_data
    URL "https://github.com/unicode-org/icu/releases/download/release-${ICU_VER}/icu4c-${ICU_VER}-data-bin-${ICU_ENDIAN_CHAR}.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(icu_data)

# Path to .dat file (assume icudt78l.dat is at the root after unzip)
set(ICU_DAT_FILENAME "icudt${ICU_VER_MAJOR}${ICU_ENDIAN_CHAR}.dat")
set(ICU_DAT_PATH "${icu_data_SOURCE_DIR}/${ICU_DAT_FILENAME}")

# 3. Generate Python script for dat -> c conversion
set(PY_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dat_to_c.py")

# 4. Custom command for data conversion
set(ICU_DAT_C_FILE "${CMAKE_BINARY_DIR}/${ICU_DAT_FILENAME}.c")
set(ICU_DAT_SYMBOL "icudt${ICU_VER_MAJOR}_dat") # icudt78_dat

add_custom_command(
    OUTPUT "${ICU_DAT_C_FILE}"
    COMMAND "${Python_EXECUTABLE}" "${PY_SCRIPT_PATH}" "${ICU_DAT_PATH}" "${ICU_DAT_C_FILE}" "${ICU_DAT_SYMBOL}"
    DEPENDS "${ICU_DAT_PATH}" "${PY_SCRIPT_PATH}"
    COMMENT "Generating C source from ICU data (${ICU_DAT_FILENAME})..."
)

# 5. Helper function to parse sources.txt
function(read_icu_sources_txt output_var list_file base_dir)
    if(NOT EXISTS "${list_file}")
        message(FATAL_ERROR "Source list not found: ${list_file}")
    endif()
    
    file(STRINGS "${list_file}" lines)
    set(files "")
    foreach(line ${lines})
        # Remove comments
        string(REGEX REPLACE "#.*$" "" line "${line}")
        string(STRIP "${line}" line)
        if(line)
            list(APPEND files "${base_dir}/${line}")
        endif()
    endforeach()
    set(${output_var} ${files} PARENT_SCOPE)
endfunction()

# ========== icudata ==========
add_library(icudata "${ICU_DAT_C_FILE}")
target_include_directories(icudata PRIVATE $<BUILD_INTERFACE:${ICU_SOURCE_DIR}/common>)

# ========== icuuc ==========
read_icu_sources_txt(UC_SRCS "${ICU_SOURCE_DIR}/common/sources.txt" "${ICU_SOURCE_DIR}/common")
add_library(icuuc ${UC_SRCS})
file(GLOB UC_HEADERS "${ICU_SOURCE_DIR}/common/unicode/*.h")
target_include_directories(icuuc PUBLIC $<BUILD_INTERFACE:${ICU_SOURCE_DIR}/common>)
target_link_libraries(icuuc PUBLIC icudata)
target_compile_definitions(icuuc PRIVATE U_COMMON_IMPLEMENTATION)

# ========== icui18n ==========
read_icu_sources_txt(I18N_SRCS "${ICU_SOURCE_DIR}/i18n/sources.txt" "${ICU_SOURCE_DIR}/i18n")
add_library(icui18n ${I18N_SRCS})
file(GLOB I18N_HEADERS "${ICU_SOURCE_DIR}/i18n/unicode/*.h")
target_include_directories(icui18n PUBLIC $<BUILD_INTERFACE:${ICU_SOURCE_DIR}/i18n>)
target_link_libraries(icui18n PUBLIC icuuc)
target_compile_definitions(icui18n PRIVATE U_I18N_IMPLEMENTATION)

# ========== icuio ==========
read_icu_sources_txt(IO_SRCS "${ICU_SOURCE_DIR}/io/sources.txt" "${ICU_SOURCE_DIR}/io")
add_library(icuio ${IO_SRCS})
file(GLOB IO_HEADERS "${ICU_SOURCE_DIR}/io/unicode/*.h")
target_include_directories(icuio PUBLIC $<BUILD_INTERFACE:${ICU_SOURCE_DIR}/io>)
target_link_libraries(icuio PUBLIC icui18n)
target_compile_definitions(icuio PRIVATE U_IO_IMPLEMENTATION)

# ========== icutu ==========
read_icu_sources_txt(UT_SRCS "${ICU_SOURCE_DIR}/tools/toolutil/sources.txt" "${ICU_SOURCE_DIR}/tools/toolutil")
add_library(icutu ${UT_SRCS})
file(GLOB UT_HEADERS "${ICU_SOURCE_DIR}/tools/toolutil/*.h")
target_include_directories(icutu PUBLIC $<BUILD_INTERFACE:${ICU_SOURCE_DIR}/tools/toolutil>)
target_link_libraries(icutu PUBLIC icui18n)
target_compile_definitions(icutu PRIVATE U_TOOLUTIL_IMPLEMENTATION)

# ========== Common settings (MSVC, static) ==========
foreach(target icuuc icui18n icuio)
    if(MSVC)
        target_compile_options(${target} PRIVATE "/utf-8")
        target_compile_definitions(${target} PRIVATE "_CRT_SECURE_NO_WARNINGS")
    endif()
    if(NOT BUILD_SHARED_LIBS)
        target_compile_definitions(${target} PUBLIC U_STATIC_IMPLEMENTATION)
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE "-w")
    endif()
endforeach()

# ========== Aliases (for find_package(ICU) style) ==========
add_library(ICU::data ALIAS icudata)
add_library(ICU::uc ALIAS icuuc)
add_library(ICU::i18n ALIAS icui18n)
add_library(ICU::io ALIAS icuio)
add_library(ICU::tu ALIAS icutu)