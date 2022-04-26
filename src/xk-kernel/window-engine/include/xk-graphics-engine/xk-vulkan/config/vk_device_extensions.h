//
// Created by kafka on 2/11/2022.
//

#ifndef XK_VK_DEVICE_EXTENSIONS_H
#define XK_VK_DEVICE_EXTENSIONS_H

#include <vector>

#include <vulkan/vulkan.hpp>

namespace xk::graphics_engine::vulkan
{
    static std::vector<const char*> DeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}

#endif //XK_VK_DEVICE_EXTENSIONS_H
