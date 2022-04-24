add_subdirectory(window-engine)
add_subdirectory(core)

add_library(xk_kernel INTERFACE)

target_link_libraries(xk_kernel INTERFACE core window_engine)

target_compile_options(xk_kernel INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
