# CMakeLists.txt in root/src

set(src_files algorithmRectangle.c compare.c main.c mkdir_p.c readCSV.c tube.c tubeSize.c)
set(hdr_files algorithmRectangle.h compare.h mkdir_p.h readCSV.h tube.h tubeSize.h)

message("Project will be compiled from the following source and header files:")
foreach(f ${src_files} ${hdr_files})
    message(${f})
endforeach()

# Add library with both header and source files.
# https://stackoverflow.com/questions/36174499/why-add-header-files-into-add-library-add-executable-command-in-cmake
add_library(lib_obj OBJECT "${src_files}" "${hdr_files}")

# Add lib and exe.
add_library(lib_shr SHARED $<TARGET_OBJECTS:lib_obj>)

# Set target properties and install.
set_target_properties(
    lib_shr
    PROPERTIES
		OUTPUT_NAME ${PROJECT_NAME}
        WINDOWS_EXPORT_ALL_SYMBOLS 1  # required to build dll (equivalent to __declspec(export) in code)
        # PUBLIC_HEADER compare.h  # if implicit linking needed + proper include directory if no install
)
install(
	TARGETS lib_shr
	RUNTIME DESTINATION "${LIBRARY_OUTPUT_DIRECTORY}"
	LIBRARY DESTINATION "${LIBRARY_OUTPUT_DIRECTORY}"
	ARCHIVE DESTINATION "${ARCHIVE_OUTPUT_DIRECTORY}"
	# PUBLIC_HEADER DESTINATION "${PUBLIC_INCLUDE_DIRECTORY}/${PROJECT_NAME}"  # if implicit linking needed
)
