file(GLOB_RECURSE sources "src/*.cpp")

add_library(window_engine STATIC ${sources})

target_include_directories(window_engine PUBLIC include PRIVATE src)

target_link_libraries(window_engine PUBLIC external)

target_compile_definitions(window_engine PRIVATE -DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_options(window_engine PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
