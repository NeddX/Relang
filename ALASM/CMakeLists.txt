# Fetch all the source and header files and the then add them automatically
file(GLOB_RECURSE ALASM_SOURCES "src/*.cpp")
file(GLOB_RECURSE ALASM_HEADERS "src/*.h")

add_library(ALASM-static STATIC ${ALASM_SOURCES} ${ALA_HEADERS})
add_executable(ALASM ${ALASM_SOURCES} ${ALA_HEADERS})

# Set the C++ Standard to 20 for this target
set_property(TARGET ALASM-static PROPERTY CXX_STANDARD 20)
set_property(TARGET ALASM PROPERTY CXX_STANDARD 20)

#
target_link_libraries(ALASM-static ALVM-static)
target_link_libraries(ALASM ALVM-static)
target_include_directories(ALASM-static PRIVATE ALVM-static)

# =============== #
#       MISC      #
# =============== #

# Make the base include file public
target_include_directories(ALASM-static PUBLIC include/)

# Add the precompiled header
#target_include_directories(ALASM-static PUBLIC pch/)
#target_precompile_headers(ALASM-static PRIVATE pch/sdafx.h)

# BS
#target_link_libraries(CodexEngine-static ${CMAKE_DL_LIBS})
#target_link_libraries(CodexEngine ${CMAKE_DL_LIBS})

# Treat all warnings as errors for better code
if (MSVC)
	#target_compile_options(ALASM-static PRIVATE /W3 /WX)
else()
	#target_compile_options(ALASM-static PRIVATE -Wall -Wextra -Wno-error=extra -Werror)
endif()
