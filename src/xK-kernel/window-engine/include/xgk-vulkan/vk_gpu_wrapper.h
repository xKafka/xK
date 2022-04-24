//
// Created by kafka on 2/6/2022.
//

#ifndef XGK_GK_VK_INSTANCE_H
#define XGK_GK_VK_INSTANCE_H

#include <memory>
#include <vector>
#include <optional>

#include <vulkan/vulkan.hpp>
#include <window/win_main_window.h>

namespace xgk::vulkan
{
    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        std::optional<u32> graphicsFamily;
        std::optional<u32> presentFamily;

        [[nodiscard]] inline auto isComplete() const -> bool { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    template<bool ValidationLayersEnabled>
    class GpuWrapper
    {
        /** This class represents the only one Vulkan instance and GPU wrapper
         *    this class is responsible for communication with GPU using VkPhysical device aka GPU
         *    for this purpose has implemented physical queue, graphics queue and command pool
         */

        using ParentWindow = xk::win::MainWindow;

        QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& gpu, vk::QueueFlags operationsToBeSupported) const;


        u32 findMemoryType(u32 typeFilter, vk::MemoryPropertyFlagBits properties);

        // helper functions

        std::vector<const char *> getRequiredExtensions() const;

        bool areDeviceExtensionsSupported(const vk::PhysicalDevice& gpu) const;

        SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& gpu);

        bool areLayersSupported(const std::vector<const char*>& layers);

        bool areExtensionsSupported(const std::vector<const char*>& extensions);

        bool isGpuGood(const vk::PhysicalDevice& gpu, vk::QueueFlags operationsToBeSupported) const;

        QueueFamilyIndices findPhysicalQueueFamilies();

        vk::Format findSupportedFormat(const std::vector<vk::Format>& formats, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

        std::tuple<vk::Buffer, vk::DeviceMemory> createVertexBuffer(u64 size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

        vk::CommandBuffer beginSingleTimeCommands();

        void endSingleTimeCommands(vk::CommandBuffer* commandBuffer);

        void copyBuffer(const vk::Buffer& src, vk::Buffer* dst, u64 size);

        void copyBufferToImage(const vk::Buffer& src, vk::Image* image, u32 width, u32 height, u32 layerCount);

        std::tuple<vk::Image, vk::DeviceMemory> createImageWithInfo(const vk::ImageCreateInfo& info, vk::MemoryPropertyFlagBits properties);

        void cleanUp();

        //Setup stages
        void createVulkanInstance();

        void setupDebugger();

        void createPresentationSurface();

        void pickGpu();

        void createLogicalDevice();

        void createCommandPool();

    public:
        explicit GpuWrapper(std::weak_ptr<ParentWindow> parent);

        static constexpr GpuWrapper createInstance(const ParentWindow& parent)
        {
            return GpuWrapper{ parent };
        }

        ~GpuWrapper();

        void init();

        [[nodiscard]] inline auto& getCommandPool() const { return m_commandPool; }

        [[nodiscard]] inline auto& getLogicalDevice() const { return m_logicalDevice; }

        [[nodiscard]] inline auto& getSurface() const { return m_surface; }

        [[nodiscard]] inline auto& getGraphicsQueue() const { return m_graphicsQueue; }

        [[nodiscard]] inline auto& getPresentQueue() const{ return m_presentQueue; }

    private:
        //VkInstance is a gateway to all the Vulkan functions
        vk::UniqueInstance              m_instance{};

        //Debugger used for debug logging
        vk::DebugUtilsMessengerEXT      m_debugMessenger{};

        //Used as a C function loader
        vk::DispatchLoaderDynamic       m_dynamicDispatch{};

        //Software representation of gpu itself
        vk::PhysicalDevice              m_gpu{};

        //Holds all the information about gpu
        vk::PhysicalDeviceProperties    m_gpuProperties{};

        //Interface to gpu
        vk::Device                      m_logicalDevice{};

        //Used to manage the allocation of vk::CommandBuffers the commands will be submitted  to Queues
        vk::CommandPool                 m_commandPool{};

        //Surface for presentation { in this program it will be GLFW surface }
        vk::SurfaceKHR                  m_surface{};

        //Queues associated to the vk::Device all the work will be submitted here
        vk::Queue                       m_graphicsQueue{}, m_presentQueue{};

        //Shared pointer to GLFW type window that we are presenting on
        std::weak_ptr<ParentWindow>     m_window;
    };

    extern template class GpuWrapper<true>;
    extern template class GpuWrapper<false>;
}

#endif //XGK_GK_VK_INSTANCE_H
