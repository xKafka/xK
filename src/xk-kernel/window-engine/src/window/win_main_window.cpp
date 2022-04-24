//
// Created by kafka on 2/3/2022.
//

#include <window/win_main_window.h>

namespace xk::win
{

     template class MainWindow<graphics_engine::gl::Core<true>, true>;
     template class MainWindow<graphics_engine::gl::Core<false>, true>;

     template class MainWindow<graphics_engine::vulkan::Core<true>, true>;
     template class MainWindow<graphics_engine::vulkan::Core<false>, true>;

}