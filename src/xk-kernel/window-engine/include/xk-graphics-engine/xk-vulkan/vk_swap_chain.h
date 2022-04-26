 
//Created by kafka on 3/17/2022.

#ifndef XK_VK_SWAP_CHAIN_H
#define XK_VK_SWAP_CHAIN_H

#include <vulkan/vulkan.h>

#include <utility/cast.h>

#include <string>
#include <vector>
#include <memory>

#include "vk_gpu_wrapper.h"

namespace xk::graphics_engine::vulkan
{
    template<bool ValidationLayersEnabled> class Core;
}

namespace xk::win
{
    template<typename GraphicsApi, bool IsValidationEnabled> class MainWindow;
}

namespace xk::graphics_engine::vulkan
{
    template<bool ValidationEnabled>
    class SwapChain
    {
        using InstanceT = GpuWrapper<ValidationEnabled>;

        using ParentWindow = win::MainWindow<Core<ValidationEnabled>, ValidationEnabled>;

        static constexpr Size_t ImagesInFlight{ 2 };

        VkFormat findDepthFormat();

        VkResult acquireNextImage(Index_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, Index_t *imageIndex);

        //Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

    public:
        explicit SwapChain(std::weak_ptr<GpuWrapper<ValidationEnabled>> gpuWrapper);

        ~SwapChain();

        void setupForWindow(std::weak_ptr<ParentWindow> parent);

        [[nodiscard]] inline auto& getFrameBuffer(Index_t index) const
        {
            m_swapChainFramebuffers[index];
        }

        [[nodiscard]] inline auto& getRenderPass() const
        {
            return m_renderPass;
        }

        [[nodiscard]] inline auto& getImageView(Index_t index) const
        {
            return m_swapChainImageViews[index];
        }

        [[nodiscard]] inline Size_t imageCount() const
        {
            return m_swapChainImages.size();
        }

        [[nodiscard]] inline auto getSwapChainImageFormat() const
        {
            return m_swapChainImageFormat;
        }

        [[nodiscard]] inline auto getSwapChainExtent() const
        {
            return m_swapChainExtent;
        }

        [[nodiscard]] inline u32 width() const
        {
            return m_swapChainExtent.width;
        }

        [[nodiscard]] inline u32 height() const
        {
            return m_swapChainExtent.height;
        }

        [[nodiscard]] inline f32 extentAspectRatio() const
        {
            return to_f32(m_swapChainExtent.width) / to_f32(m_swapChainExtent.height);
        }

    private:
        std::weak_ptr<InstanceT>    m_gpuWrapper;
        VkSwapchainKHR              m_swapChain;

        VkFormat                    m_swapChainImageFormat{};
        VkExtent2D                  m_swapChainExtent, m_windowExtent{};

        VkRenderPass                m_renderPass{};

        std::vector<VkFramebuffer>  m_swapChainFramebuffers{};
        std::vector<VkImage>        m_depthImages{};
        std::vector<VkDeviceMemory> m_depthImageMemories{};
        std::vector<VkImageView>    m_depthImageViews{};
        std::vector<VkImage>        m_swapChainImages{};
        std::vector<VkImageView>    m_swapChainImageViews{};
        std::vector<VkSemaphore>    m_imageAvailableSemaphores{};
        std::vector<VkSemaphore>    m_renderFinishedSemaphores{};
        std::vector<VkFence>        m_inFlightFences{};
        std::vector<VkFence>        m_imagesInFlight{};

        Index_t                     m_currentFrame{ 0 };
    };
}
 
#endif  XK_VK_SWAP_CHAIN_H
