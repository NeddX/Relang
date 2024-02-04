cmake_minimum_required(VERSION 3.20)

project("Blend")

include_directories(include/)

# Set some macros
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

# Sub projects
add_subdirectory("blend")
add_subdirectory("basm")
#add_subdirectory("alcc")