project("alcc")

set(ALCC_OUTPUT_DIR "${CMAKE_BINARY_DIR}/${PROJECT_NAME}")
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)

# Call 'cargo build' to build the Rust project
if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_target(
        alcc-rs ALL
        COMMAND cargo build --target-dir "${ALCC_OUTPUT_DIR}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_custom_command(
        TARGET alcc-rs
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ALCC_OUTPUT_DIR}/debug/" "${BLEND_BINARY_OUTPUT_DIR}"
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_target(
        alcc-rs ALL
        COMMAND cargo build --target-dir "${ALCC_OUTPUT_DIR}" --release
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_custom_command(
        TARGET alcc-rs
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ALCC_OUTPUT_DIR}/debug/" "${BLEND_BINARY_OUTPUT_DIR}"
    )
endif()
