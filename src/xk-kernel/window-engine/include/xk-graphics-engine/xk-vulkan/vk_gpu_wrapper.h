//
// Created by kafka on 2/6/2022.
//

#ifndef XK_VK_INSTANCE_H
#define XK_VK_INSTANCE_H

#include <memory>
#include <vector>
#include <optional>

#include <vulkan/vulkan.hpp>

#include <utility/literal.h>

namespace xk::graphics_engine::vulkan
{
   template<bool ValidationLayersEnabled> class Core;
}

namespace xk::win
{
    template<typename GraphicsApiHandler, bool IsValidationEnabled> class MainWindow;
}

namespace xk::graphics_engine::vulkan
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR>   presentModes{};
    };

    struct QueueFamilyIndices
    {
        std::optional<u32> graphicsFamily{};
        std::optional<u32> presentFamily{};

        [[nodiscard]] inline auto isComplete() const -> bool { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    template<bool ValidationLayersEnabled>
    class GpuWrapper
    {
        /** This class represents the only one Vulkan instance and GPU wrapper
         *    this class is responsible for communication with GPU using VkPhysical device aka GPU
         *    for this purpose has implemented physical queue, graphics queue and command pool
         */

        using ParentWindow = win::MainWindow<Core<ValidationLayersEnabled>, ValidationLayersEnabled>;

        static constexpr auto EmptyGenericValue = VK_NULL_HANDLE;

        [[nodiscard]] auto findMemoryType(u32 typeFilter, VkMemoryPropertyFlagBits properties) const -> u32;

        [[nodiscard]] auto findQueueFamilies(const VkPhysicalDevice& gpu, VkQueueFlags operationsToBeSupported) const -> QueueFamilyIndices;

        // helper functions
        [[nodiscard]] auto areDeviceExtensionsSupported(const VkPhysicalDevice& gpu, const std::vector<const char*>& deviceExtensions) const -> bool;

        [[nodiscard]] auto querySwapChainSupport(const VkPhysicalDevice& gpu) const -> SwapChainSupportDetails;

        [[nodiscard]] auto areGlfwExtensionsSupported(const std::vector<const char*>& glfwExtensions) const -> bool;

        [[nodiscard]] auto areValidationLayersSupported(const std::vector<const char*>& validationLayers) const -> bool;

        [[nodiscard]] auto isGpuGood(const VkPhysicalDevice& gpu, VkQueueFlags operationsToBeSupported) const -> bool;

        [[nodiscard]] auto findPhysicalQueueFamilies(VkQueueFlags operationsToBeSupported) const -> QueueFamilyIndices;

        [[nodiscard]] std::vector<const char*> requiredGlfwExtensions() const;

        auto findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) -> VkFormat;

        auto createVertexBuffer(u64 size, VkBufferUsageFlags usage, VkMemoryPropertyFlagBits properties) -> std::tuple<VkBuffer, VkDeviceMemory>;

        auto beginSingleTimeCommands() -> VkCommandBuffer;

        auto endSingleTimeCommands(VkCommandBuffer* commandBuffer) -> void;

        auto copyBuffer(const VkBuffer& src, VkBuffer* dst, u64 size) -> void;

        auto copyBufferToImage(const VkBuffer& src, VkImage* image, u32 width, u32 height, u32 layerCount) -> void;

        auto createImageWithInfo(const VkImageCreateInfo& info, VkMemoryPropertyFlagBits properties) -> std::tuple<VkImage, VkDeviceMemory>;

        void cleanUp();

        //Setup stages
        void createVulkanInstance();

        void setupDebugger();

        void createPresentationSurface();

        void pickGpu();

        void createLogicalDevice();

        void createCommandPool();

    public:
        explicit GpuWrapper();

        ~GpuWrapper();

        static std::shared_ptr<GpuWrapper> createGpuWrapper();

        void setupForWindow(std::weak_ptr<ParentWindow> parent);

        [[nodiscard]] inline auto& getCommandPool() const { return m_commandPool; }

        [[nodiscard]] inline auto& getLogicalDevice() const { return m_logicalDevice; }

        [[nodiscard]] inline auto& getSurface() const { return m_surface; }

        [[nodiscard]] inline auto& getGraphicsQueue() const { return m_graphicsQueue; }

        [[nodiscard]] inline auto& getPresentQueue() const{ return m_presentQueue; }

        [[nodiscard]] inline auto getSwapChainSupport() const { return querySwapChainSupport(m_gpu); }

        [[nodiscard]] inline auto findPhysicalQueueFamilies() const { return findQueueFamilies(m_gpu); }

    private:
        //VkInstance is a gateway to all the Vulkan functions
        VkInstance                      m_instance{ EmptyGenericValue };

        //Debugger used for debug logging
        VkDebugUtilsMessengerEXT        m_debugMessenger{ EmptyGenericValue };

        //Software representation of gpu itself
        VkPhysicalDevice                m_gpu{ EmptyGenericValue };

        //Holds all the information about gpu
        VkPhysicalDeviceProperties      m_gpuProperties{ };

        //Interface to gpu
        VkDevice                        m_logicalDevice{ EmptyGenericValue };

        //Used to manage the allocation of VkCommandBuffers the commands will be submitted  to Queues
        VkCommandPool                   m_commandPool{ EmptyGenericValue };

        //Surface for presentation { in this program it will be GLFW surface }
        VkSurfaceKHR                    m_surface{ EmptyGenericValue };

        //Queues associated to the VkDevice all the work will be submitted here
        VkQueue                         m_graphicsQueue{ EmptyGenericValue }, m_presentQueue{ EmptyGenericValue };

        //Shared pointer to GLFW type window that we are presenting on
        std::weak_ptr<ParentWindow>     m_window{};
    };

    extern template class GpuWrapper<true>;
    extern template class GpuWrapper<false>;
}

#endif //XK_VK_INSTANCE_H
