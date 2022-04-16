//
// Created by kafka on 2/6/2022.
//

#ifndef XGK_GK_VK_VALIDATION_CALLBACK_H
#define XGK_GK_VK_VALIDATION_CALLBACK_H

#include <vulkan/vulkan.hpp>
#include <utility/literal.h>
#include <spdlog/spdlog.h>

namespace xgk::vulkan
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        switch(messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            {
                spdlog::info("Validation layer diagnostic message: [{}]", pCallbackData->pMessage);
            }
            break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            {
                spdlog::info("Validation layer informational message [{}]", pCallbackData->pMessage);
            }
            break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            {
                spdlog::warn("Validation layer possible bug report [{}]", pCallbackData->pMessage);
            }
            break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            {
                spdlog::error("Validation layer invalid behaviour [{}]", pCallbackData->pMessage);
            }
            break;
            default:
            {
                spdlog::error("Validation layer: uknown message severity");
            }
        }

        switch(messageType)
        {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            {
                spdlog::warn("Validation layer event unrelated to the specific performance [{}]", pCallbackData->pMessage);
            }
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            {
                spdlog::warn("Validation layer specification or performance violation [{}]", pCallbackData->pMessage);
            }
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            {
                spdlog::warn("Validation layer potential non-optimal use of Vulkan [{}]", pCallbackData->pMessage);
            }
            default:
            {
                spdlog::error("Validation layer: unknown message type");
            }
        }

        return VK_FALSE;
     }

    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
}

#endif //XGK_GK_VK_VALIDATION_CALLBACK_H
