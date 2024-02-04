project("refront")

file(GLOB_RECURSE REF_SRC_FILES "src/*.cpp")
file(GLOB_RECURSE REF_HDR_FILES "src/*.h")

add_executable(refront ${REF_SRC_FILES} ${CMC_HDR_FILES})

set_property(TARGET refront PROPERTY CXX_STANDARD 20)

target_include_directories(refront PRIVATE "include/")

target_link_libraries(refront blend-static)

add_subdirectory("vendor/fmt" ${CMAKE_BINARY_DIR}/fmt)
set(FMT_INCLUDE_DIRS "vendor/fmt/include")
set(FMT_LIBRARIES fmt)
target_link_libraries(refront ${FMT_LIBRARIES})
target_include_directories(refront PRIVATE ${FMT_INCLUDE_DIRS})

add_subdirectory("vendor/nlohmann_json" ${CMAKE_BINARY_DIR}/nlohmann_json)
set(NLOHMANN_JSON_INCLUDE_DIRS "vendor/nlohmann_json/include")
set(NLOHMANN_JSON_LIBRARIES nlohmann_json)
target_link_libraries(refront ${NLOHMANN_JSON_LIBRARIES})
target_include_directories(refront PRIVATE ${NLOHMANN_JSON_INCLUDE_DIRS})

install(TARGETS refront DESTINATION bin)
