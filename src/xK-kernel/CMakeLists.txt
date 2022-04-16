add_subdirectory(window-engine)
add_subdirectory(core)

add_library(xgk INTERFACE)

target_link_libraries(xgk INTERFACE core window_engine)

target_compile_options(xgk INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
