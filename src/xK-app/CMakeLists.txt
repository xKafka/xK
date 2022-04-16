add_executable(xgk_app main.cpp)

target_include_directories(xgk_app PRIVATE xgk "${CMAKE_BINARY_DIR}/configured_files/include")

target_link_libraries(xgk_app xgk)