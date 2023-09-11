cmake_minimum_required(VERSION 3.20)

project("ALVM")

if (DEFINED CMAKE_BUILD_TYPE)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
		elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O0")
		endif()	
	elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
		if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
		elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O3")
		endif()
	endif()
else()
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O0")
	endif()
endif()


#if (UNIX)
#   if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_C_COMPILER_ID MATCHES "Clang" OR
#   	CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_C_COMPILER_ID MATCHES "GNU")
#   	message(" :: Disabling annoying harmless warnings... ")
#   	message(" :: NOTE: REMEMBER TO TURN THEM BACK ON FOR QUALITY CONTROL.")
#   	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c99-designator -Wno-shift-op-parentheses -Wno-switch-enum")
#   	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-c99-designator -Wno-shift-op-parentheses -Wno-switch-enum")
#   endif()
#endif()

# Set some macros--
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")
set(ALVM_BINARY_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/")
set(ALVM_LIBRARY_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/lib/")

# Create the bin/ and lib/ folders in the root directory.
add_custom_target(build-time-config ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ALVM_BINARY_OUTPUT_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${ALVM_LIBRARY_OUTPUT_DIR}
)

# Sub projects
add_subdirectory("alvm")
add_subdirectory("alasm")
add_subdirectory("alcc")

# Copy compile commands to the root directory.
add_custom_command(
	TARGET build-time-config
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}"
)
