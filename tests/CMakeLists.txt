# CMakeLists.txt in root/tests

include_directories(${LIBRARY_OUTPUT_DIRECTORY})  # not enough on Windows: lib still needs to stand next to exe

add_executable(test_lib EXCLUDE_FROM_ALL test_lib.c) 

set_property(
    TARGET test_lib 
    PROPERTY 
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/$<CONFIG>"
)

target_link_libraries(test_lib lib_shr)
if(MACOSX OR LINUX)
    target_link_libraries(test_lib dl)
endif()

add_custom_target(compile_test)
add_dependencies(compile_test test_lib)