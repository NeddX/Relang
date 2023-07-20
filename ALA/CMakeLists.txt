# Fetch all the source and header files and the then add them automatically
file(GLOB_RECURSE ALA_SOURCES "src/*.cpp")
file(GLOB_RECURSE ALA_HEADERS "src/*.h")

add_library(ALA-static STATIC ${ALA_SOURCES} ${ALA_HEADERS})

# Set the C++ Standard to 20 for this target
set_property(TARGET ALA-static PROPERTY CXX_STANDARD 20)

#
target_link_libraries(ALA-static ALVM-static)
target_include_directories(ALA-static PRIVATE ALVM-static)

# =============== #
#       MISC      #
# =============== #

# Make the base include file public
target_include_directories(ALA-static PUBLIC include/)

# Add the precompiled header
#target_include_directories(ALA-static PUBLIC pch/)
#target_precompile_headers(ALA-static PRIVATE pch/sdafx.h)

# BS
#target_link_libraries(CodexEngine-static ${CMAKE_DL_LIBS})
#target_link_libraries(CodexEngine ${CMAKE_DL_LIBS})

# Treat all warnings as errors for better code
if (MSVC)
	#target_compile_options(ALA-static PRIVATE /W3 /WX)
else()
	#target_compile_options(ALA-static PRIVATE -Wall -Wextra -Wno-error=extra -Werror)
endif()
