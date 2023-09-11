project("alvm")

# Fetch all the source and header files and the then add them automatically
file(GLOB_RECURSE ALVM_SOURCES "src/*.cpp")
file(GLOB_RECURSE ALVM_HEADERS "src/*.h")

add_library(alvm-static STATIC ${ALVM_SOURCES} ${ALVM_HEADERS})
add_executable(alvm ${ALVM_SOURCES} ${ALVM_HEADERS})
#add_library(alvm-static-lib STATIC ${ALVM_SOURCES} ${ALVM_HEADERS})

#set_property(TARGET alvm-static-lib PROPERTY CXX_STANDARD 20)

# =============== #
#       MISC      #
# =============== #


# Make the base include file public
target_include_directories(alvm-static PUBLIC include/)
#target_include_directories(alvm-static-lib PUBLIC include/)

# Add the precompiled header
target_include_directories(alvm-static PUBLIC pch/)
target_include_directories(alvm PUBLIC pch/)
target_precompile_headers(alvm-static PRIVATE pch/sdafx.h)
target_precompile_headers(alvm PRIVATE pch/sdafx.h)
#target_include_directories(alvm-static-lib PUBLIC pch/)
#target_precompile_headers(alvm-static-lib PRIVATE pch/sdafx.h)

# Treat all warnings as errors for better code
if (MSVC)
	#target_compile_options(alvm-static PRIVATE /W3 /WX)
else()
	#target_compile_options(alvm-static PRIVATE -Wall -Wextra -Wno-error=extra -Werror)
endif()

# Copy the compiled binary and library to bin/ and lib/ directories.
add_custom_command(TARGET alvm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:alvm> ${ALVM_BINARY_OUTPUT_DIR}
    #COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:alvm-shared> ${ALVM_LIBRARY_OUTPUT_DIR}
)
