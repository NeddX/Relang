cmake_minimum_required(VERSION 3.20)

project("ALVM")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_BUILD_TYPE Debug)
set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor/")

add_subdirectory("ALVM")
add_subdirectory("ALA")
add_subdirectory("Test")
