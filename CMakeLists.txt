cmake_minimum_required(VERSION 3.20)

project("Blend")

include_directories(include/)

# Set some macros
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

# Create a custom build command and a dummy target for it to copy
# compile_commands.json to the source directory for LSPs.
add_custom_command(
  OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json
          ${CMAKE_SOURCE_DIR}/compile_commands.json
  DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json...")

add_custom_target(copy_compile_commands ALL
                  DEPENDS ${CMAKE_SOURCE_DIR}/compile_commands.json)

# Sub projects
add_subdirectory("blend")
add_subdirectory("basm")
add_subdirectory("refront")
#add_subdirectory("alcc")
