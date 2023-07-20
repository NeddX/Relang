file(GLOB_RECURSE ALVM_TEST_SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE ALVM_TEST_HEADER_FILES "src/*.h")

add_executable(rtest ${ALVM_TEST_HEADER_FILES} ${ALVM_TEST_SOURCE_FILES})

target_link_libraries(rtest ALVM-static ALA-static)
target_include_directories(rtest PRIVATE ALVM-static ALA-static)
