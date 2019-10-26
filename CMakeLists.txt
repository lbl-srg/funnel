# CMakeLists.txt in root
#
# @directions:
#   Build from root/build/. with commands:
#   > cmake .. (add `-A x64` on Windows to compile in 64 bits, MSVC compiles in 32 bits by default)
#   > cmake --build . --target install (add `--config Release` on Windows, MSVC compiles in Debug mode by default)
#   > ctest (add `-C Release` on Windows + add -R {lib, py or num} to specify a specific set of tests)
#
# @bug
#   *   Library test crashes on x86 due to readCSV(90, 91) realloc invalid next size
#   *   Adapt PLATFORM_INSTALL_PREFIX to build x86 binary on x64 host: no built-in CMake variable for TARGET architecture
#   *   CMake CMAKE_SYSTEM_PROCESSOR in tool chain file: error at build time
#       see https://stackoverflow.com/questions/54151877/cmake-cmake-system-processor-in-tool-chain-file-error-at-build-time
#
# @todo
#   Add test with Python 3
#   Temporary test dir for parallel running

cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.11.0)
project(funnel VERSION 0.1.0 LANGUAGES C)
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
    message(WARNING "OS unknown!")
endif()

# Output build system directories.
message("CMake source directory is: ${CMAKE_SOURCE_DIR}")
message("CMake binary directory is: ${CMAKE_BINARY_DIR}")

# Set compiling and linking options.
# Consider adding -std=c89 -pedantic to gcc options to improve portability.
# Note: -std=c99 is to support the for loops syntax when compiling with gcc on centOS.
set(UNIX_COMPILE_FLAGS "-Wall -Wfloat-equal -o3 -fPIC -std=c99")
set(WIN_COMPILE_FLAGS "/Wall /O2")
set(UNIX_LINK_FLAGS "-fPIC")

if(LINUX OR MACOSX)  # CMake 3.13.*: support for add_compile_options and add_link_options
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${UNIX_COMPILE_FLAGS}")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${UNIX_LINK_FLAGS}")
else()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WIN_COMPILE_FLAGS}")
endif()

# Set target directories.
# NOTA: always add quotes to protect spaces in path when setting new variables.
if (WINDOWS)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES ".*64$")
        set(PLATFORM_INSTALL_PREFIX win64)
    else()
        set(PLATFORM_INSTALL_PREFIX win32)
    endif()
elseif(LINUX)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES ".*64$")
        set(PLATFORM_INSTALL_PREFIX linux64)
    else()
        set(PLATFORM_INSTALL_PREFIX linux32)
    endif()
elseif(MACOSX)
    set(PLATFORM_INSTALL_PREFIX darwin64)
endif()

set(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${PLATFORM_INSTALL_PREFIX}")
set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/${PLATFORM_INSTALL_PREFIX}")
set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(PUBLIC_INCLUDE_DIRECTORY "${CMAKE_SOURCE_DIR}/include")

# Manage subdirectories.
add_subdirectory(src)
add_subdirectory(tests)

############################################################
# TESTS
############################################################

# Manage tests.
include(FindPythonInterp)
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
    COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/bin/pyfunnel.py" ${TEST_ARGS_0}
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
### Wrong arguments
set(TEST_ARGS_1 --reference wrong.csv --test simulated.csv --atolx 0.002 --atoly 0.002 --output results)
add_test(
    NAME test_py_1
    COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/bin/pyfunnel.py" ${TEST_ARGS_1}
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
set_tests_properties(test_py_1 PROPERTIES PASS_REGULAR_EXPRESSION "No such file")
### Again
set(TEST_ARGS_2 --reference trended.csv --test simulated.csv --atoly 0.002 --output results)
add_test(
    NAME test_py_2
    COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/bin/pyfunnel.py" ${TEST_ARGS_2}
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
set_tests_properties(test_py_2 PROPERTIES PASS_REGULAR_EXPRESSION "At least one of the two possible tolerance parameters")
### Again
set(TEST_ARGS_3 --reference trended.csv --test simulated.csv --atolx 0.002 --atoly 0.002)
add_test(
    NAME test_py_3
    COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/bin/pyfunnel.py" ${TEST_ARGS_3}
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
set_tests_properties(test_py_3 PROPERTIES PASS_REGULAR_EXPRESSION "Output directory not specified")

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
    set(CTEST_CUSTOM_PRE_TEST "${CTEST_CUSTOM_PRE_TEST}
        \"${CMAKE_COMMAND} -E make_directory ${tmp_dir}\""
    )
    add_test(
        NAME ${test_name}
        COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_TEST_DIR}/test_numerics.py" ${test_name} ${test_dir} ${tmp_dir}
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
    COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_TEST_DIR}/test_plot.py" "${CMAKE_TEST_DIR}/fail1/results"
    WORKING_DIRECTORY "${CMAKE_TEST_DIR}/test_bin"
)
set_tests_properties(test_plot PROPERTIES FAIL_REGULAR_EXPRESSION "Exception;Error;Traceback")

## Configure pre/post test.
set(content "# Numerics testing configuration file
    set(CTEST_CUSTOM_PRE_TEST
        ${CTEST_CUSTOM_PRE_TEST})"
)
file(WRITE
    "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
    "${content}"
)