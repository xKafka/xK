//
// Created by kafka on 2/6/2022.
//

#ifndef XGK_GK_VK_EXCEPTIONS_H
#define XGK_GK_VK_EXCEPTIONS_H

#include <stdexcept>
#include <string>

#include <xgk-vulkan/vk_resul_wrappert.h>

namespace xgk::vulkan
{
    struct Exception : public std::runtime_error
    {
        using Parent = std::runtime_error;

        explicit Exception(std::string_view message)
            : Parent{ message.data() }
        {}

        static Exception PipelineError(std::string_view message)
        {
            return Exception{ std::string{"Pipeline error: "} + message.data() };
        }

        static Exception InstanceError(const std::string& message, const VkResult result)
        {
            return Exception{ "GpuWrapper error: " + message + " error code: { "+ getVulkanErrorName(result).data() + " }" };
        }

        static Exception InstanceError(const std::string& message)
        {
            return Exception{ "GpuWrapper error: " + message };
        }

        static Exception GPUError(const std::string& message, const VkResult result)
        {
            return Exception{ "GPU error: " + message + " error code: { " + getVulkanErrorName(result).data() + " }" };
        }

        static Exception GPUError(const std::string& message)
        {
            return Exception{ "GPU error: " + message };
        }
    };
}

#endif //XGK_GK_VK_EXCEPTIONS_H
