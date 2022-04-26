find_package(Vulkan REQUIRED)

add_subdirectory(fmt-8.1.1)
add_subdirectory(spdlog-1.x)
add_subdirectory(glfw-3.3.6)

add_library(external INTERFACE)

target_include_directories(external INTERFACE ${Vulkan_INCLUDE_DIRS})

target_link_libraries(external INTERFACE glfw spdlog::spdlog fmt::fmt Vulkan::Vulkan)