# Fetch all the source and header files and the then add them automatically
file(GLOB_RECURSE ALVM_SOURCES "src/*.cpp")
file(GLOB_RECURSE ALVM_HEADERS "src/*.h")

add_library(ALVM-static STATIC ${ALVM_SOURCES} ${ALVM_HEADERS})
add_executable(ALVM ${ALVM_SOURCES} ${ALVM_HEADERS})
#add_library(ALVM-static-lib STATIC ${ALVM_SOURCES} ${ALVM_HEADERS})

#set_property(TARGET ALVM-static-lib PROPERTY CXX_STANDARD 20)

# =============== #
#       MISC      #
# =============== #

# Make the base include file public
target_include_directories(ALVM-static PUBLIC include/)
#target_include_directories(ALVM-static-lib PUBLIC include/)

# Add the precompiled header
target_include_directories(ALVM-static PUBLIC pch/)
target_include_directories(ALVM PUBLIC pch/)
target_precompile_headers(ALVM-static PRIVATE pch/sdafx.h)
target_precompile_headers(ALVM PRIVATE pch/sdafx.h)
#target_include_directories(ALVM-static-lib PUBLIC pch/)
#target_precompile_headers(ALVM-static-lib PRIVATE pch/sdafx.h)

# Treat all warnings as errors for better code
if (MSVC)
	#target_compile_options(ALVM-staticPRIVATE /W3 /WX)
else()
	#target_compile_options(ALVM-staticPRIVATE -Wall -Wextra -Wno-error=extra -Werror)
endif()
