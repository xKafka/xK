//
// Created by kafka on 2/11/2022.
//

#ifndef XK_VK_VALIDATION_LAYERS_H
#define XK_VK_VALIDATION_LAYERS_H

#include <vector>

namespace xk::graphics_engine::vulkan
{
    static std::vector<const char*> ValidationLayers{ "VK_LAYER_KHRONOS_validation" };
}

#endif //XK_VK_VALIDATION_LAYERS_H
