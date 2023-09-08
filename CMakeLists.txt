cmake_minimum_required(VERSION 3.20)

project("ALVM")

if (DEFINED CMAKE_BUILD_TYPE)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
		endif()	
	elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
		if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
			message(" :: OPTIMIZATION")
		endif()
	endif()
else()
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor/")

add_subdirectory("ALVM")
add_subdirectory("ALASM")
