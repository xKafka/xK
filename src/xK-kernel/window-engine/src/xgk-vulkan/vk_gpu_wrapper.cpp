//
// Created by kafka on 2/6/2022.
//

#include <set>
#include <ranges>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <xgk-vulkan/vk_gpu_wrapper.h>
#include <xgk-vulkan/vk_exceptions.h>
#include <xgk-vulkan/vk_validation_callback.h>
#include <xgk-vulkan/config/vk_validation_layers.h>
#include <xgk-vulkan/config/vk_device_extensions.h>

#include <application/app_info.h>
#include <utility/log.h>

#define VULKAN_HPP_NO_CONSTRUCTORS

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace xgk::vulkan
{
    static constexpr auto VulkanSuccess{ VK_SUCCESS };

    template<bool ValidationLayersEnabled>
    GpuWrapper<ValidationLayersEnabled>::GpuWrapper(std::weak_ptr<ParentWindow> parent)
            : m_window{ parent }
    {
        init();
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::init()
    {
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
        //dynamicFunctorsLoader
        vk::DynamicLoader dfl;

        auto vkGetInstanceProcAddr = dfl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        if constexpr(ValidationLayersEnabled)
        {
            static const std::vector Layers
            {
                "VK_LAYER_KHRONOS_validation"
            };

            if(areLayersSupported(Layers))
            {
                throw Exception::InstanceError("Validation layers are not supported");
            }
        }

        static constexpr vk::ApplicationInfo applicationInfo
        {
            xgk::application::info::AppName.data(),
            VK_MAKE_VERSION(1,0,0),
            "Best Framework Ever",
            VK_MAKE_VERSION(1,0,0),
            VK_API_VERSION_1_0
        };

        auto extensions = m_window.lock()->requiredWindowExtensions();

        if constexpr(ValidationLayersEnabled)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        if(areExtensionsSupported(extensions))
        {
            throw Exception::InstanceError("Extensions are not supported");
        }

        vk::InstanceCreateInfo instanceCreateInfo
        {
                {},
                &applicationInfo,
                0,
                {},
                static_cast<u32>(extensions.size()),
                extensions.data()
        };

        if constexpr(ValidationLayersEnabled)
        {
            using Severenity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
            using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

            static vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo
            {
                .messageSeverity =  Severenity::eVerbose | Severenity::eError | Severenity::eWarning,
                .messageType = Type::eGeneral | Type::ePerformance | Type::eValidation,
                .pfnUserCallback = debugCallback,
                .pUserData = nullptr
            };

            instanceCreateInfo.setEnabledLayerCount(static_cast<u32>(ValidationLayers.size()));
            instanceCreateInfo.setPpEnabledLayerNames(ValidationLayers.data());
            instanceCreateInfo.setPNext((VkDebugUtilsMessengerCreateInfoEXT*) &debugMessengerCreateInfo);
        }

        m_instance = vk::createInstanceUnique(instanceCreateInfo);

        VULKAN_HPP_DEFAULT_DISPATCHER.init( m_instance.get() );

        if(!m_instance)
        {
            throw Exception::InstanceError("GpuWrapper could not be created");
        }
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::setupDebugger()
    {
        using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        using MessageType = vk::DebugUtilsMessageTypeFlagBitsEXT;

        static constexpr vk::DebugUtilsMessengerCreateInfoEXT createInfoExt
        {
                {},
                Severity::eError,
                MessageType::eGeneral | MessageType::ePerformance | MessageType::eValidation,
                debugCallback,
                nullptr
        };

        m_debugMessenger = m_instance.get().createDebugUtilsMessengerEXT(createInfoExt, nullptr, VULKAN_HPP_DEFAULT_DISPATCHER);

        if (!m_debugMessenger)
        {
            throw Exception::InstanceError("debug messenger create");
        }
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::createPresentationSurface()
    {
        m_surface = m_window.lock()->createVulkanWindowSurface(m_instance.get());
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::pickGpu()
    {
        const auto gpus = m_instance.get().enumeratePhysicalDevices();

        if (gpus.empty())
        {
            throw Exception::InstanceError("failed to find GPUs with Vulkan support!");
        }

        static constexpr vk::QueueFlags supportedOperations = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;

        log::info("Device count: {}", gpus.size());

        for (const vk::PhysicalDevice &gpu : gpus)
        {
            if (isGpuGood(gpu, supportedOperations))
            {
                m_gpu = gpu;

                break;
            }
        }

        if (!m_gpu)
        {
            throw Exception::InstanceError("failed to find a suitable GPU!");
        }

        m_gpuProperties = m_gpu.getProperties();

        log::info("Physical device: {}", m_gpuProperties.deviceName);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::createLogicalDevice()
    {
        static constexpr vk::QueueFlags supportedOperations = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;

        //get queue families with graphics and presentation support
        QueueFamilyIndices indices = findQueueFamilies(m_gpu, supportedOperations);

        //sort with graphics support first
        std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfos.reserve(uniqueQueueFamilies.size());

        static constexpr f32 priority = 1.0f;

        for(const u32 queueFamilyIndex : uniqueQueueFamilies)
        {
            auto queueCreateInfo = vk::DeviceQueueCreateInfo{}
                    .setQueueFamilyIndex(queueFamilyIndex)
                    .setQueueCount(1u)
                    .setPQueuePriorities(&priority);

            queueCreateInfos.push_back(queueCreateInfo);
        }

        const auto deviceDesiredFeatures = vk::PhysicalDeviceFeatures{}
                .setSamplerAnisotropy(VK_TRUE);

        auto deviceCreateInfo = vk::DeviceCreateInfo{}
                .setQueueCreateInfoCount(static_cast<u32>(queueCreateInfos.size()))
                .setPQueueCreateInfos(queueCreateInfos.data())
                .setEnabledExtensionCount(static_cast<u32>(DeviceExtensions.size()))
                .setPpEnabledExtensionNames(DeviceExtensions.data())
                .setPEnabledFeatures(&deviceDesiredFeatures);

        if constexpr(ValidationLayersEnabled)
        {
            deviceCreateInfo.setEnabledLayerCount(static_cast<u32>(ValidationLayers.size()));
            deviceCreateInfo.setPpEnabledLayerNames(ValidationLayers.data());
        }
        else
        {

            deviceCreateInfo.setEnabledLayerCount(0);
        }

        m_logicalDevice = m_gpu.createDevice(deviceCreateInfo);

        if(!m_logicalDevice)
        {
            throw Exception::InstanceError("failed to create logical device!");
        }

        m_graphicsQueue = m_logicalDevice.getQueue(indices.graphicsFamily.value(), 0);
        m_presentQueue = m_logicalDevice.getQueue(indices.presentFamily.value(), 0);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::createCommandPool()
    {
        static constexpr vk::QueueFlags supportedOperations = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;

        const auto queueFamilyIndices = findQueueFamilies(m_gpu, supportedOperations);

        const auto poolInfo = vk::CommandPoolCreateInfo{}
        .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

        m_commandPool = m_logicalDevice.createCommandPool(poolInfo);

        if(!m_commandPool)
        {
            throw Exception::InstanceError("failed to create command pool!");
        }
    }

    template<bool ValidationLayersEnabled>
    bool GpuWrapper<ValidationLayersEnabled>::areLayersSupported(const std::vector<const char*>& layers)
    {
        const auto availableLayers = vk::enumerateInstanceLayerProperties();

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

        for(const auto& layer : layers)
        {
            if(!isLayerSupported(layer))
            {
                return false;
            }
        }

        return true;
    }

    template<bool ValidationLayersEnabled>
    bool GpuWrapper<ValidationLayersEnabled>::areExtensionsSupported(const std::vector<const char*>& extensions)
    {
        const auto availableExtensions = vk::enumerateInstanceExtensionProperties();

        if(availableExtensions.empty())
        {
            throw Exception::InstanceError("could not enumerate layers ");
        }

        auto isExtensionSupported = [&](std::string_view extensionName) -> bool
        {
            return std::ranges::any_of(availableExtensions, [&](const auto& extension)
            {
                return extensionName == std::string_view{ extension.extensionName };
            });
        };

        for(const auto& extension : extensions)
        {
            if(!isExtensionSupported(extension))
            {
                return false;
            }
        }

        return true;
    }

    template<bool ValidationLayersEnabled>
    void /*!Done */
    GpuWrapper<ValidationLayersEnabled>::createSurface()
    {
        m_surface = m_window.lock()->createVulkanWindowSurface(m_instance.get());
    }

    template<bool ValidationLayersEnabled>
    bool /*!Done */
    GpuWrapper<ValidationLayersEnabled>::isGpuGood(const vk::PhysicalDevice& gpu, vk::QueueFlags operationsToBeSupported) const
    {
        const auto indices = findQueueFamilies(gpu, operationsToBeSupported);

        const bool extensionsSupported = areDeviceExtensionsSupported(gpu);

        bool swapChainAdequate = false;

        if (extensionsSupported)
        {
            const auto swapChainSupport = querySwapChainSupport(gpu);

            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        const auto supportedDeviceFeatures = gpu.getFeatures();

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedDeviceFeatures.samplerAnisotropy;
    }

    template<bool ValidationLayersEnabled>
    std::vector<const char *> /*! Done*/
    GpuWrapper<ValidationLayersEnabled>::getRequiredExtensions() const
    {
        u32 glfwExtensionCount = 0;

        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if constexpr (ValidationLayersEnabled)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    template<bool ValidationLayersEnabled>
    bool /*! Done*/
    GpuWrapper<ValidationLayersEnabled>::areDeviceExtensionsSupported(const vk::PhysicalDevice& gpu) const
    {
        const auto availableExtensions = gpu.enumerateDeviceExtensionProperties();

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

        for(const auto& extension : DeviceExtensions)
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
    GpuWrapper<ValidationLayersEnabled>::findQueueFamilies(const vk::PhysicalDevice& gpu, const vk::QueueFlags operationsToBeSupported) const
    {
        QueueFamilyIndices indices;

        const auto queueFamilies = gpu.getQueueFamilyProperties();

        for(u32 i{ 0 }; i < queueFamilies.size(); ++i)
        {
            const auto& familyProperties = queueFamilies.at(i);

            if(familyProperties.queueCount > 0 && (familyProperties.queueFlags & operationsToBeSupported))
            {
                indices.graphicsFamily = std::make_optional(i);
            }

            const auto presentSupport = gpu.getSurfaceSupportKHR(i, m_surface);

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
    SwapChainSupportDetails /*! Done*/
    GpuWrapper<ValidationLayersEnabled>::querySwapChainSupport(const vk::PhysicalDevice& gpu)
    {
        SwapChainSupportDetails details{};

        details.capabilities = gpu.getSurfaceCapabilitiesKHR(m_surface);

        details.formats = gpu.getSurfaceFormatsKHR(m_surface);

        details.presentModes = gpu.getSurfacePresentModesKHR(m_surface);

        return details;
    }

    template<bool ValidationLayersEnabled>
    vk::Format /*! Done*/
    GpuWrapper<ValidationLayersEnabled>::findSupportedFormat(const std::vector<vk::Format>& formats, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
   {
        for (const auto& format : candidates)
        {
            const auto properties = m_gpu.getFormatProperties(format);

            if(tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw Exception::InstanceError("GpuWrapper::findSupportedFormat -> failed to find supported format!");
    }

    template<bool ValidationLayersEnabled>
    u32
    GpuWrapper<ValidationLayersEnabled>::findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_gpu, &memProperties);

        for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw Exception::InstanceError("failed to find suitable memory type!");
    }

    template<bool ValidationLayersEnabled>
    std::tuple<vk::Buffer, vk::DeviceMemory>
    GpuWrapper<ValidationLayersEnabled>::createVertexBuffer(u64 size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    {
        const auto bufferCreateInfo = vk::BufferCreateInfo{}
        .setSize(size)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive);

        vk::Buffer vertexBuffer = m_logicalDevice.createBuffer(bufferInfo);

        if (!vertexBuffer)
        {
            throw Exception::InstanceError("failed to create vertex buffer!");
        }

        const auto memoryRequirements = m_logicalDevice.getBufferMemoryRequirements(vertexBuffer);

        const auto allocateInfo = vk::MemoryAllocateInfo{}
        .setAllocationSize(memoryRequirements.size)
        .setMemoryTypeIndex(memoryRequirements.memoryTypeBits, properties);

        vk::DeviceMemory bufferMemory = m_logicalDevice.allocateMemory(allocateInfo);

        if (!bufferMemory)
        {
            throw Exception::InstanceError("failed to allocate vertex buffer memory!");
        }

        m_logicalDevice.bindBufferMemory(vertexBuffer, bufferMemory, 0);

        return { vertexBuffer, bufferMemory };
    }

    template<bool ValidationLayersEnabled>
    VkCommandBuffer
    GpuWrapper<ValidationLayersEnabled>::beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        allocInfo.commandPool = m_commandPool;

        VkCommandBuffer commandBuffer;

        vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

        constexpr VkCommandBufferBeginInfo beginInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
        };

        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphicsQueue);

        vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion
        {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
        };

        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::copyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height, u32 layerCount)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region
        {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageOffset = { 0, 0, 0 },
            .imageExtent = { width, height, 1 }
        };

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory)
    {
        if (vkCreateImage(m_logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw Exception::InstanceError("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
        };

        if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw Exception::InstanceError("failed to allocate image memory!");
        }

        if (vkBindImageMemory(m_logicalDevice, image, imageMemory, 0) != VK_SUCCESS)
        {
            throw Exception::InstanceError("failed to bind image memory!");
        }
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::cleanUp()
    {
        vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);

        vkDestroyDevice(m_logicalDevice, nullptr);

        if constexpr (ValidationLayersEnabled)
        {
            cleanDebugMessenger();
        }

        vkDestroySurfaceKHR(m_instance.get(), m_surface, nullptr);

        vkDestroyInstance(m_instance.get(), nullptr);
    }

    template<bool ValidationLayersEnabled>
    void
    GpuWrapper<ValidationLayersEnabled>::cleanDebugMessenger()
    {
        destroyDebugUtilsMessengerEXT(m_instance.get(), m_debugMessenger, nullptr);
    }

    template class GpuWrapper<true>;
    template class GpuWrapper<false>;
}