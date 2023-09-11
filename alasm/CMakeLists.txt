project("alasm")

# Fetch all the source and header files and the then add them automatically
file(GLOB_RECURSE ALASM_SOURCES "src/*.cpp")
file(GLOB_RECURSE ALASM_HEADERS "src/*.h")

add_library(alasm-static STATIC ${ALASM_SOURCES} ${ALA_HEADERS})
add_executable(alasm ${ALASM_SOURCES} ${ALA_HEADERS})

# Set the C++ Standard to 20 for this target
set_property(TARGET alasm-static PROPERTY CXX_STANDARD 20)
set_property(TARGET alasm PROPERTY CXX_STANDARD 20)

#
target_link_libraries(alasm-static alvm-static)
target_link_libraries(alasm alvm-static)
target_include_directories(alasm-static PRIVATE alvm-static)

# =============== #
#       MISC      #
# =============== #

# Make the base include file public
target_include_directories(alasm-static PUBLIC include/)

# Add the precompiled header
#target_include_directories(alasm-static PUBLIC pch/)
#target_precompile_headers(alasm-static PRIVATE pch/sdafx.h)

# BS
#target_link_libraries(CodexEngine-static ${CMAKE_DL_LIBS})
#target_link_libraries(CodexEngine ${CMAKE_DL_LIBS})

# Treat all warnings as errors for better code
if (MSVC)
	#target_compile_options(alasm-static PRIVATE /W3 /WX)
else()
	#target_compile_options(alasm-static PRIVATE -Wall -Wextra -Wno-error=extra -Werror)
endif()

# Copy the compiled binary and library to bin/ and lib/ directories.
add_custom_command(TARGET alasm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:alasm> ${ALVM_BINARY_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:alasm-static> ${ALVM_LIBRARY_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:alvm-static> ${ALVM_LIBRARY_OUTPUT_DIR}
)
