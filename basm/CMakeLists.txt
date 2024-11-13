project("basm")

# Fetch all the source and header files and the then add them automatically
file(GLOB_RECURSE BASM_SOURCES "src/*.cpp")
file(GLOB_RECURSE BASM_HEADERS "src/*.h")

add_library(basm-static STATIC ${BASM_SOURCES} ${ALA_HEADERS})
add_executable(basm ${BASM_SOURCES} ${ALA_HEADERS})

# Set the C++ Standard to 20 for this target
set_property(TARGET basm-static PROPERTY CXX_STANDARD 20)
set_property(TARGET basm PROPERTY CXX_STANDARD 20)

#
target_link_libraries(basm-static blend-static)
target_link_libraries(basm blend-static)
target_include_directories(basm-static PRIVATE blend-static)

# =============== #
#       MISC      #
# =============== #

# Make the base include file public
target_include_directories(basm-static PUBLIC include/)

install(TARGETS basm DESTINATION bin/)
install(TARGETS basm-static DESTINATION lib/)

# Add the precompiled header
#target_include_directories(basm-static PUBLIC pch/)
#target_precompile_headers(basm-static PRIVATE pch/sdafx.h)

# Treat all warnings as errors for better code
if (MSVC)
	#target_compile_options(basm-static PRIVATE /W3 /WX)
else()
	#target_compile_options(basm-static PRIVATE -Wall -Wextra -Wno-error=extra -Werror)
endif()

# ======================= # INSTALLATION # ======================= #
install(TARGETS basm-static DESTINATION lib)
install(TARGETS basm DESTINATION bin)
