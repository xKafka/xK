//
// Created by kafka on 2/11/2022.
//

#ifndef XGK_VK_DEVICE_EXTENSIONS_H
#define XGK_VK_DEVICE_EXTENSIONS_H

#include <array>
#include <vulkan/vulkan.hpp>

namespace xgk::vulkan
{
    static constexpr std::array DeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}

#endif //XGK_VK_DEVICE_EXTENSIONS_H
