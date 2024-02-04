project("blend")

# Fetch all the source and header files and the then add them automatically
file(GLOB_RECURSE BLEND_SOURCES "src/*.cpp")
file(GLOB_RECURSE BLEND_HEADERS "src/*.h")

add_library(blend-static STATIC ${BLEND_SOURCES} ${BLEND_HEADERS})
add_executable(blend ${BLEND_SOURCES} ${BLEND_HEADERS})

# Set the C++ Standard to 20 for this target
set_property(TARGET blend-static PROPERTY CXX_STANDARD 20)
set_property(TARGET blend PROPERTY CXX_STANDARD 20)

# =============== # MISC # =============== #
# Make the base include file public
target_include_directories(blend-static PUBLIC include/)
# target_include_directories(blend-static-lib PUBLIC include/)

# Add the precompiled header
target_include_directories(blend-static PUBLIC pch/)
target_include_directories(blend PUBLIC pch/)
target_precompile_headers(blend-static PRIVATE pch/sdafx.h)
target_precompile_headers(blend PRIVATE pch/sdafx.h)

# Treat all warnings as errors for better code
if(MSVC)
  # target_compile_options(blend-static PRIVATE /W3 /WX)
else()
  # target_compile_options(blend-static PRIVATE -Wall -Wextra -Wno-error=extra
  # -Werror)
endif()

# ======================= # INSTALLATION # ======================= #
install(TARGETS blend-static DESTINATION lib)
install(TARGETS blend DESTINATION bin)