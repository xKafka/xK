//
// Created by kafka on 2/6/2022.
//

#include <set>
#include <ranges>
#include <cassert>

#include <application/app_info.h>
#include <utility/log.h>
#include <window/win_main_window.h>

#include <xk-graphics-engine/xk-vulkan/vk_gpu_wrapper.h>

#include <xk-graphics-engine/xk-vulkan/vk_core.h>
#include <xk-graphics-engine/xk-vulkan/vk_exceptions.h>
#include <xk-graphics-engine/xk-vulkan/vk_validation_callback.h>
#include <xk-graphics-engine/xk-vulkan/config/vk_validation_layers.h>
#include <xk-graphics-engine/xk-vulkan/config/vk_device_extensions.h>

namespace xk::graphics_engine::vulkan
{
    template<bool ValidationLayersEnabled>
    GpuWrapper<ValidationLayersEnabled>::GpuWrapper()
    {}

    template<bool ValidationLayersEnabled>
    std::shared_ptr<GpuWrapper<ValidationLayersEnabled>>
    GpuWrapper<ValidationLayersEnabled>::createGpuWrapper()
    {
        return std::make_shared<GpuWrapper<ValidationLayersEnabled>>();
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::setupForWindow(std::weak_ptr<ParentWindow> parent)
    {
        m_window = parent;

        createVulkanInstance();
        setupDebugger();
        createPresentationSurface();
        pickGpu();
        createLogicalDevice();
        createCommandPool();
    }

    template<bool ValidationLayersEnabled>
    GpuWrapper<ValidationLayersEnabled>::~GpuWrapper()
    {
        cleanUp();
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::createVulkanInstance()
    {
        if constexpr(ValidationLayersEnabled)
        {
            if(!areValidationLayersSupported(ValidationLayers))
            {
                throw Exception::InstanceError("Validation layers are not supported");
            }
        }

        static constexpr VkApplicationInfo applicationInfo
        {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = application::info::AppName.data(),
            .applicationVersion = VK_MAKE_VERSION(1,0,0),
            .pEngineName        = "Best Framework Ever",
            .engineVersion      = VK_MAKE_VERSION(1,0,0),
            .apiVersion         = VK_API_VERSION_1_0
        };

        auto extensions = requiredGlfwExtensions();

        VkInstanceCreateInfo instanceCreateInfo
        {
            .sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo           = &applicationInfo,
            .enabledExtensionCount      = static_cast<u32>(extensions.size()),
            .ppEnabledExtensionNames    = extensions.data()
        };

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
        };

        if constexpr (ValidationLayersEnabled)
        {
            debugCreateInfo.messageSeverity         = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType             = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback         = debugCallback;
            debugCreateInfo.pUserData               = nullptr;  // Optional


            instanceCreateInfo.enabledLayerCount    = static_cast<u32>(ValidationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames  = ValidationLayers.data();
            instanceCreateInfo.pNext                = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
        }
        else
        {
            instanceCreateInfo.enabledLayerCount    = 0;
            instanceCreateInfo.pNext                = nullptr;
        }

        if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance) != VK_SUCCESS)
        {
            throw Exception::InstanceError("GpuWrapper could not be created");
        }

        if (!areGlfwExtensionsSupported(requiredGlfwExtensions()))
        {
            throw Exception::InstanceError("Glfw extensions are not supported");
        }
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::setupDebugger()
    {
        static constexpr VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = nullptr  // Optional
        };

        if (CreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        {
            throw Exception::InstanceError("debug messenger create");
        }
    }

    template<bool ValidationLayersEnabled>
    std::vector<const char*>
    GpuWrapper<ValidationLayersEnabled>::requiredGlfwExtensions() const
    {
        if constexpr(ValidationLayersEnabled)
        {
            log::info("Obtaining required window extensions");
        }

        u32 extensionCount = 0;

        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        std::vector<const char*> returnValue(extensions, extensions + extensionCount);

        if constexpr(ValidationLayersEnabled)
        {
            returnValue.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            log::info("Extensions:");

            for(const auto& name : returnValue)
            {
                log::info("{}", name);
            }
        }

        if(returnValue.empty())
        {
            throw Exception::InstanceError("Could not obtain extensions");
        }

        return returnValue;
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::createPresentationSurface()
    {
        m_surface = m_window.lock()->createVulkanWindowSurface(m_instance);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::pickGpu()
    {
        u32 deviceCount = 0;

        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw Exception::InstanceError("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> gpus(deviceCount);

        vkEnumeratePhysicalDevices(m_instance, &deviceCount, gpus.data());

        static constexpr auto supportedOperations = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

        log::info("Device count: {}", gpus.size());

        for (const VkPhysicalDevice &gpu : gpus)
        {
            if (isGpuGood(gpu, supportedOperations))
            {
                m_gpu = gpu;

                break;
            }
        }

        if (m_gpu == VK_NULL_HANDLE)
        {
            throw Exception::InstanceError("failed to find a suitable GPU!");
        }

        vkGetPhysicalDeviceProperties(m_gpu, &m_gpuProperties);

        log::info("Physical device: {}", m_gpuProperties.deviceName);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::createLogicalDevice()
    {
        static constexpr auto supportedOperations = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

        //get queue families with graphics and presentation support
        QueueFamilyIndices indices = findQueueFamilies(m_gpu, supportedOperations);

        //sort with graphics support first
        std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        queueCreateInfos.reserve(uniqueQueueFamilies.size());

        static constexpr f32 priority = 1.0f;

        for (const u32 queueFamilyIndex : uniqueQueueFamilies)
        {
            queueCreateInfos.emplace_back(VkDeviceQueueCreateInfo
            {
                .sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex   = queueFamilyIndex,
                .queueCount         = 1,
                .pQueuePriorities   = &priority
            });
        }

        const VkPhysicalDeviceFeatures deviceDesiredFeatures
        {
            .samplerAnisotropy = VK_TRUE
        };

        VkDeviceCreateInfo deviceCreateInfo
        {
            .sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount       = static_cast<u32>(queueCreateInfos.size()),
            .pQueueCreateInfos          = queueCreateInfos.data(),
            .enabledExtensionCount      = static_cast<u32>(DeviceExtensions.size()),
            .ppEnabledExtensionNames    =  DeviceExtensions.data(),
            .pEnabledFeatures           =  &deviceDesiredFeatures
        };

        if constexpr(ValidationLayersEnabled)
        {
            deviceCreateInfo.enabledLayerCount = static_cast<u32>(ValidationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
        }
        else
        {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_gpu, &deviceCreateInfo, nullptr, &m_logicalDevice) != VK_SUCCESS)
        {
            throw Exception::InstanceError("failed to create logical device!");
        }

        vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);

        vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::createCommandPool()
    {
        static constexpr auto supportedOperations = VK_QUEUE_GRAPHICS_BIT;

        const auto queueFamilyIndices = findQueueFamilies(m_gpu, supportedOperations);

        VkCommandPoolCreateInfo poolInfo
        {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags              = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex   = queueFamilyIndices.graphicsFamily.value()
        };

        if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        {
            throw Exception::InstanceError("failed to create command pool!");
        }
    }

    template<bool ValidationLayersEnabled>
    bool
    GpuWrapper<ValidationLayersEnabled>::areValidationLayersSupported(const std::vector<const char*>& validationLayers) const
    {
        u32 layerCount = 0;

        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);

        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        if(availableLayers.empty())
        {
            throw Exception::InstanceError("could not enumerate layers ");
        }

        auto isLayerSupported = [&](std::string_view layerName) -> bool
        {
            return std::ranges::any_of(availableLayers, [&](const auto& layer)
            {
                return layerName == std::string_view{ layer.layerName };
            });
        };

        for(const auto& neededLayer : validationLayers)
        {
            if(!isLayerSupported(neededLayer))
            {
                return false;
            }
        }

        return true;
    }

    template<bool ValidationLayersEnabled>
    bool
    GpuWrapper<ValidationLayersEnabled>::areGlfwExtensionsSupported(const std::vector<const char*>& glfwExtensions) const
    {
        u32 extensionCount = 0; 

        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);

        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

        auto isExtensionSupported = [&](std::string_view extensionName) -> bool
        {
            return std::ranges::any_of(availableExtensions, [&](const auto& extension)
                {
                    return extensionName == std::string_view{ extension.extensionName };
                });
        };

        for (const auto& required : glfwExtensions)
        {
            if (!isExtensionSupported(required))
            {
                return false;
            }
        }

        return true;
    }

    template<bool ValidationLayersEnabled>
    bool /*!Done */
    GpuWrapper<ValidationLayersEnabled>::isGpuGood(const VkPhysicalDevice& gpu, VkQueueFlags operationsToBeSupported) const
    {
        const auto indices = findQueueFamilies(gpu, operationsToBeSupported);

        const bool extensionsSupported = areDeviceExtensionsSupported(gpu, DeviceExtensions);

        bool swapChainAdequate = false;

        if (extensionsSupported)
        {
            const auto swapChainSupport = querySwapChainSupport(gpu);

            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        
        vkGetPhysicalDeviceFeatures(gpu, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    template<bool ValidationLayersEnabled>
    bool /*! Done*/
    GpuWrapper<ValidationLayersEnabled>::areDeviceExtensionsSupported(const VkPhysicalDevice& gpu, const std::vector<const char*>& deviceExtensions) const
    {
        u32 extensionCount = 0;

        vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);

        vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, availableExtensions.data());

        if(availableExtensions.empty())
        {
            throw Exception::InstanceError("GpuWrapper::areDeviceExtensionsSupported -> could not get available extensions");
        }

        auto isExtensionSupported = [&](std::string_view extensionName) -> bool
        {
            return std::ranges::any_of(availableExtensions, [&](const auto& extension)
            {
                return extensionName == std::string_view{ extension.extensionName };
            });
        };

        for(const auto& extension : deviceExtensions)
        {
            if(!isExtensionSupported(extension))
            {
                return false;
            }
        }

        return true;
    }

    template<bool ValidationLayersEnabled>
    QueueFamilyIndices /*! Done*/
    GpuWrapper<ValidationLayersEnabled>::findQueueFamilies(const VkPhysicalDevice& gpu, VkQueueFlags operationsToBeSupported) const
    {
        QueueFamilyIndices indices;

        u32 queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilies.data());

        for(u32 i{ 0 }; i < queueFamilies.size(); ++i)
        {
            const auto& familyProperties = queueFamilies.at(i);

            if(familyProperties.queueCount > 0 && (familyProperties.queueFlags & operationsToBeSupported))
            {
                indices.graphicsFamily = std::make_optional(i);
            }

            VkBool32 presentSupport{ VkBool32{ 0 } };

            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, m_surface, &presentSupport);

            if(familyProperties.queueCount > 0 && presentSupport)
            {
                indices.presentFamily = std::make_optional(i);
            }

            if(indices.isComplete())
            {
                break;
            }
        }

        return indices;
    }

    template<bool ValidationLayersEnabled>
    QueueFamilyIndices /*! Done*/
    GpuWrapper<ValidationLayersEnabled>::findPhysicalQueueFamilies(const VkQueueFlags operationsToBeSupported) const
    {
        return findQueueFamilies(m_gpu, operationsToBeSupported);
    }

    template<bool ValidationLayersEnabled>
    SwapChainSupportDetails /*! Done*/
    GpuWrapper<ValidationLayersEnabled>::querySwapChainSupport(const VkPhysicalDevice& gpu) const
    {
        SwapChainSupportDetails details{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, m_surface, &details.capabilities);

        u32 formatCount{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_surface, &formatCount, nullptr);

        if(formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_surface, &formatCount, details.formats.data());
        }

        u32 presentModeCount{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, m_surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);

            vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, m_surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    template<bool ValidationLayersEnabled>
    VkFormat /*! Done*/
    GpuWrapper<ValidationLayersEnabled>::findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features)
   {
        for (const auto& format : formats)
        {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(m_gpu, format, &properties);

            if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw Exception::InstanceError("GpuWrapper::findSupportedFormat -> failed to find supported format!");
    }

    template<bool ValidationLayersEnabled>
    u32
    GpuWrapper<ValidationLayersEnabled>::findMemoryType(u32 typeFilter, VkMemoryPropertyFlagBits properties) const
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(m_gpu, &memoryProperties);

        for (u32 i{ 0 }; i < memoryProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw Exception::InstanceError("failed to find suitable memory type!");
    }

    template<bool ValidationLayersEnabled>
    std::tuple<VkBuffer, VkDeviceMemory>
    GpuWrapper<ValidationLayersEnabled>::createVertexBuffer(u64 size, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits properties)
    {
        VkBufferCreateInfo bufferInfo
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VkBuffer vertexBuffer{};

        //returning allocated object,
        if(vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
        {
            throw Exception::InstanceError("failed to create vertex buffer!");
        }

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(m_logicalDevice, vertexBuffer, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
        };

        VkDeviceMemory bufferMemory{};

        if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            throw Exception::InstanceError("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(m_logicalDevice, vertexBuffer, bufferMemory, 0);

        return { vertexBuffer, bufferMemory };
    }

    template<bool ValidationLayersEnabled>
    VkCommandBuffer
    GpuWrapper<ValidationLayersEnabled>::beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer commandBuffer{};
        vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::endSingleTimeCommands(VkCommandBuffer* commandBuffer)
    {
        vkEndCommandBuffer(*commandBuffer);

        VkSubmitInfo submitInfo
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = commandBuffer
        };

        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

        vkQueueWaitIdle(m_graphicsQueue);

        vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, commandBuffer);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::copyBuffer(const VkBuffer& src, VkBuffer* dst, u64 size)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion
        {
            .srcOffset = 0,  // Optional
            .dstOffset = 0,  // Optional
            .size = size
        };

        vkCmdCopyBuffer(commandBuffer, src, *dst, 1, &copyRegion);

        endSingleTimeCommands(&commandBuffer);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::copyBufferToImage(const VkBuffer& src, VkImage* image, u32 width, u32 height, u32 layerCount)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region
        {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0
        };

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(commandBuffer, src, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,  &region);

        endSingleTimeCommands(&commandBuffer);
    }

    template<bool ValidationLayersEnabled>
    std::tuple<VkImage, VkDeviceMemory>
    GpuWrapper<ValidationLayersEnabled>::createImageWithInfo(const VkImageCreateInfo& info, VkMemoryPropertyFlagBits properties)
    {
        VkImage image{};

        if (vkCreateImage(m_logicalDevice, &info, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(m_logicalDevice, image, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
        };

        VkDeviceMemory imageMemory{};

        if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        if (vkBindImageMemory(m_logicalDevice, image, imageMemory, 0) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to bind image memory!");
        }

        return { image, imageMemory };
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::cleanUp()
    {
        vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
        vkDestroyDevice(m_logicalDevice, nullptr);

        if constexpr (ValidationLayersEnabled)
        {
            DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    template class GpuWrapper<true>;
    template class GpuWrapper<false>;
}