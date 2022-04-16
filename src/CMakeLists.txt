find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(docopt CONFIG)
find_package(glm CONFIG)
find_package(Vulkan CONFIG)
find_package(glfw3 CONFIG)

add_subdirectory(xK-kernel)
add_subdirectory(xK-app)

