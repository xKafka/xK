file(GLOB_RECURSE sources "src/*.cpp")

add_library(core STATIC ${sources})

target_include_directories(core PUBLIC include PRIVATE src)

target_link_libraries(core PUBLIC external)