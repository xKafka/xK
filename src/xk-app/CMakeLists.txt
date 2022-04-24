add_executable(xk_app main.cpp)

target_include_directories(xk_app PRIVATE xk_kernel "${CMAKE_BINARY_DIR}/configured_files/include")

target_link_libraries(xk_app xk_kernel)