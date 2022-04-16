file(GLOB_RECURSE sources "src/*.cpp")

add_library(core STATIC ${sources})

target_include_directories(core PUBLIC include PRIVATE src)

target_link_libraries(core PUBLIC   project_options
                                    project_warnings
                                    docopt::docopt
                                    fmt::fmt
                                    spdlog::spdlog
                                    glm::glm
                                    Vulkan::Vulkan
                                    glfw)

target_compile_definitions(core PRIVATE -DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_options(core PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-pedantic>)