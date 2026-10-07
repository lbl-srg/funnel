# CMakeLists.txt in root
#
# @directions:
#   Build from root/build/. with commands:
#   > cmake ..
#   > cmake --build . --target install (add `--config Release` on Windows, MSVC compiles in Debug mode by default)
#   > ctest --verbose (add `-C Release` on Windows + add -R {lib, py or num} to specify a specific set of tests)
#
# @bug
#   *   Library test crashes on x86 due to readCSV(90, 91) realloc invalid next size
#   *   Adapt PLATFORM_INSTALL_PREFIX to build x86 binary on x64 host: no built-in CMake variable for TARGET architecture
#   *   CMake CMAKE_SYSTEM_PROCESSOR in tool chain file: error at build time
#       see https://stackoverflow.com/questions/54151877/cmake-cmake-system-processor-in-tool-chain-file-error-at-build-time
#

cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.22)

set(VERSION "2.1.0")  # DO NOT CHANGE: this is automatically updated with 'cz bump'
message("Project version is: ${VERSION}.")

# Configure macOS universal build.
# This should be set prior to the first project() or enable_language() command invocation because
# it may influence configuration of the toolchain and flags.
# This is ignored on platforms other than Apple.
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "")

# Set up project.
project(funnel VERSION ${VERSION} LANGUAGES C)
enable_testing()

# Detect OS.
if(CMAKE_SYSTEM_NAME MATCHES Windows)
  set(WINDOWS 1)
  message("Windows system detected.")
elseif(CMAKE_SYSTEM_NAME MATCHES Linux)
  set(LINUX 1)
  message("Linux system detected.")
elseif(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(MACOSX 1)
  message("Mac OS X system detected.")
else()
  message(FATAL_ERROR "OS unknown!")
endif()

# Set compiling and linking options.
if(LINUX OR MACOSX)
  set(CMAKE_C_FLAGS "-fPIC -Wall -Wextra -Werror -Wfloat-equal -Wpedantic -O3")
  set(CMAKE_SHARED_LINKER_FLAGS "-fPIC")
else()
  set(CMAKE_C_FLAGS "/Wall /O2")
endif()

message("CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")

# Set target directories.
# NOTE: always add quotes to protect spaces in path when setting new variables.
if (WINDOWS)
  # Use the target architecture, which can differ from the host processor,
  # e.g., -A x64 on Windows ARM64. Must match the directory selected in pyfunnel/core.py.
  if(CMAKE_C_COMPILER_ARCHITECTURE_ID)  # Set by MSVC: x64, ARM64, X86.
    string(TOUPPER "${CMAKE_C_COMPILER_ARCHITECTURE_ID}" TARGET_ARCH)
  elseif(CMAKE_GENERATOR_PLATFORM)  # Set with -A for Visual Studio generators.
    string(TOUPPER "${CMAKE_GENERATOR_PLATFORM}" TARGET_ARCH)
  else()
    string(TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" TARGET_ARCH)
  endif()
  if(TARGET_ARCH MATCHES "ARM64")
    set(PLATFORM_INSTALL_PREFIX winarm64)
  elseif(TARGET_ARCH MATCHES ".*64$")
    set(PLATFORM_INSTALL_PREFIX win64)
  else()
    set(PLATFORM_INSTALL_PREFIX win32)
  endif()
elseif(LINUX)
  # Must match the directory selected in pyfunnel/core.py.
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" TARGET_ARCH)
  if(TARGET_ARCH MATCHES "^(aarch64|arm64)$")
    set(PLATFORM_INSTALL_PREFIX linuxarm64)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_INSTALL_PREFIX linux64)
  else()
    set(PLATFORM_INSTALL_PREFIX linux32)
  endif()
elseif(MACOSX)
  set(PLATFORM_INSTALL_PREFIX darwin64)
endif()

set(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/pyfunnel/lib/${PLATFORM_INSTALL_PREFIX}")
set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/pyfunnel/lib/${PLATFORM_INSTALL_PREFIX}")

# Print build system directories.
message("CMake source directory is: ${CMAKE_SOURCE_DIR}")
message("CMake binary directory is: ${CMAKE_BINARY_DIR}")
message("CMake archive directory is: ${ARCHIVE_OUTPUT_DIRECTORY}")
message("CMake library directory is: ${LIBRARY_OUTPUT_DIRECTORY}")

# Manage subdirectories.
add_subdirectory(src)
add_subdirectory(tests)

############################################################
# TESTS
############################################################

# Manage tests.
find_program(Python_EXECUTABLE NAMES python3 python REQUIRED)  # Searches $PATH, matches `which python3`.
find_package(Python REQUIRED COMPONENTS Interpreter)           # Accepts Python_EXECUTABLE as a hint, validates it.

message("Python interpreter used for testing: ${Python_EXECUTABLE}")
set(CTEST_OUTPUT_ON_FAILURE ON)  # verbose if failed test
set(CMAKE_TEST_DIR "${CMAKE_SOURCE_DIR}/tests")

## Build test exe.
add_test(
    NAME test_build_lib
    COMMAND ${CMAKE_COMMAND} --build . --target compile_test --config $<CONFIGURATION>
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
## Library testing.
add_test(
    NAME test_bin_lib
    COMMAND test_lib
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
## Python script testing.
### Base
set(TEST_ARGS_0 --reference trended.csv --test simulated.csv --atolx 0.002 --atoly 0.002 --output results)
add_test(
    NAME test_py_0
    COMMAND ${Python_EXECUTABLE} "${CMAKE_SOURCE_DIR}/pyfunnel/cli.py" ${TEST_ARGS_0}
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
### Wrong arguments
set(TEST_ARGS_1 --reference wrong.csv --test simulated.csv --atolx 0.002 --atoly 0.002 --output results)
add_test(
    NAME test_py_1
    COMMAND ${Python_EXECUTABLE} "${CMAKE_SOURCE_DIR}/pyfunnel/cli.py" ${TEST_ARGS_1}
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
set_tests_properties(test_py_1 PROPERTIES PASS_REGULAR_EXPRESSION "No such file")
### Wrong arguments
set(TEST_ARGS_2 --reference trended.csv --test simulated.csv --atolx 0.002 --atoly 0.002)
add_test(
    NAME test_py_2
    COMMAND ${Python_EXECUTABLE} "${CMAKE_SOURCE_DIR}/pyfunnel/cli.py" ${TEST_ARGS_2}
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
set_tests_properties(test_py_2 PROPERTIES PASS_REGULAR_EXPRESSION "Output directory not specified")

## Numerics testing.

file(
    GLOB TEST_CASES
    LIST_DIRECTORIES true
    RELATIVE ${CMAKE_TEST_DIR}
    ${CMAKE_TEST_DIR}/fail* ${CMAKE_TEST_DIR}/success*
)
list(LENGTH TEST_CASES len_tmp)
math(EXPR len_tests "${len_tmp} - 1")
foreach(t RANGE ${len_tests})
  list(GET TEST_CASES ${t} test_case)
  set(test_name test_numerics_${t})
  set(test_dir "${CMAKE_TEST_DIR}/${test_case}")
  set(tmp_dir "${CMAKE_TEST_DIR}/${test_name}")
  # Paths are quoted as they may contain spaces, e.g. C:/Program Files/... on Windows.
  list(APPEND PRE_TEST_COMMANDS "\"${CMAKE_COMMAND}\" -E make_directory \"${tmp_dir}\"")
  add_test(
        NAME ${test_name}
        COMMAND ${Python_EXECUTABLE} ${CMAKE_TEST_DIR}/test_numerics.py ${test_name} ${test_dir} ${tmp_dir}
        WORKING_DIRECTORY ${tmp_dir}
    )
  if (${test_case} MATCHES "fail4")  # WILL_FAIL shall not be used simultaneously with PASS_REGULAR_EXPRESSION!
    set_tests_properties(${test_name} PROPERTIES PASS_REGULAR_EXPRESSION "Failed to create directory")
  endif()
  if (${test_case} MATCHES "fail(2|6)")  # WILL_FAIL shall not be used simultaneously with PASS_REGULAR_EXPRESSION!
    set_tests_properties(${test_name} PROPERTIES PASS_REGULAR_EXPRESSION "Reference and test data maximum x values are different")
  endif()
endforeach()

## Plot function testing.
add_test(
    NAME test_plot
    COMMAND ${Python_EXECUTABLE} ${CMAKE_TEST_DIR}/test_plot.py ${CMAKE_TEST_DIR}/fail1/results
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
set_tests_properties(test_plot PROPERTIES FAIL_REGULAR_EXPRESSION "Exception;Error;Traceback")

## Library path testing.
add_test(
    NAME test_lib_path
    COMMAND ${Python_EXECUTABLE} ${CMAKE_TEST_DIR}/test_lib_path.py
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}"
)

## Local HTTP server testing.
add_test(
    NAME test_server
    COMMAND ${Python_EXECUTABLE} ${CMAKE_TEST_DIR}/test_server.py
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}"
)

## Configure pre/post test.
set(POST_TEST_COMMAND
    "\"${Python_EXECUTABLE}\" \"${CMAKE_TEST_DIR}/test_summary.py\" \"${CMAKE_TEST_DIR}\""
)
# Write each command as a quoted CMake string, escaping the quotes around paths.
set(pre_test "")
foreach(cmd IN LISTS PRE_TEST_COMMANDS)
  string(REPLACE "\"" "\\\"" cmd "${cmd}")
  string(APPEND pre_test "\n        \"${cmd}\"")
endforeach()
string(REPLACE "\"" "\\\"" post_test "${POST_TEST_COMMAND}")
set(content "# Numerics testing configuration file
    set(CTEST_CUSTOM_PRE_TEST${pre_test})
    set(CTEST_CUSTOM_POST_TEST
        \"${post_test}\")"
)
file(WRITE
    "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
    "${content}"
)
