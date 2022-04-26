 
//Created by kafka on 3/17/2022.
 

#include <xk-graphics-engine/xk-vulkan/vk_swap_chain.h>
#include <xk-graphics-engine/xk-vulkan/vk_exceptions.h>
#include <utility/log.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace xk::graphics_engine::vulkan
{
    template<bool ValidationEnabled>
    SwapChain<ValidationEnabled>::SwapChain(std::weak_ptr<GpuWrapper<ValidationEnabled>> gpuWrapper)
        : m_gpuWrapper{ gpuWrapper }
    {
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDepthResources();
        createFramebuffers();
        createSyncObjects();
    }

    template<bool ValidationEnabled>
    SwapChain<ValidationEnabled>::~SwapChain()
    {
        for (auto imageView : m_swapChainImageViews)
        {
            vkDestroyImageView(m_gpuWrapper. m_gpuWrapper(), imageView, nullptr);
        }

        m_swapChainImageViews.clear();

        if (m_swapChain != nullptr)
        {
            vkDestroySwapchainKHR(m_gpuWrapper. m_gpuWrapper(), m_swapChain, nullptr);

            m_swapChain = nullptr;
        }

        for (Size_t i{ 0 }; i < m_depthImages.size(); ++i)
        {
            vkDestroyImageView(m_gpuWrapper. m_gpuWrapper(), m_depthImageViews[i], nullptr);
            vkDestroyImage(m_gpuWrapper. m_gpuWrapper(), m_depthImages[i], nullptr);
            vkFreeMemory(m_gpuWrapper. m_gpuWrapper(), m_depthImageMemories[i], nullptr);
        }

        for (auto framebuffer : m_swapChainFramebuffers)
        {
            vkDestroyFramebuffer(m_gpuWrapper. m_gpuWrapper(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(m_gpuWrapper. m_gpuWrapper(), m_renderPass, nullptr);

        for (Size_t i{ 0 }; i < ImagesInFlight; ++i)
        {
            vkDestroySemaphore(m_gpuWrapper. m_gpuWrapper(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_gpuWrapper. m_gpuWrapper(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_gpuWrapper. m_gpuWrapper(), m_inFlightFences[i], nullptr);
        }
    }

    template<bool ValidationEnabled>
    VkResult
    SwapChain<ValidationEnabled>::acquireNextImage(Index_t *imageIndex)
    {
        vkWaitForFences(m_gpuWrapper. m_gpuWrapper(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

        VkResult result = vkAcquireNextImageKHR(m_gpuWrapper. m_gpuWrapper(), m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, imageIndex);

        return result;
     }

    template<bool ValidationEnabled>
    VkResult
    SwapChain<ValidationEnabled>::submitCommandBuffers(const VkCommandBuffer *buffers, Index_t *imageIndex)
    {
        if (m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(m_gpuWrapper. m_gpuWrapper(), 1, &m_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
        }

        m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };

        VkSubmitInfo submitInfo
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = buffers,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores
        };

        vkResetFences(m_gpuWrapper. m_gpuWrapper(), 1, &m_inFlightFences[m_currentFrame]);

        if (vkQueueSubmit(m_gpuWrapper.graphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        {
            throw Exception::SwapChainError("failed to submit draw command buffer!");
        }

        VkSwapchainKHR swapChains[] = { m_swapChain };

        VkPresentInfoKHR presentInfo
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = imageIndex
        };

        auto result = vkQueuePresentKHR(m_gpuWrapper.presentQueue(), &presentInfo);

        m_currentFrame = (m_currentFrame + 1) % ImagesInFlight;

        return result;
     }

    template<bool ValidationEnabled>
    void
    SwapChain<ValidationEnabled>::createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport =  m_gpuWrapper.getSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        Size_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        QueueFamilyIndices indices =  m_gpuWrapper.findPhysicalQueueFamilies();

        std::array queueFamilyIndices { indices.graphicsFamily.value(), indices.presentFamily.value() };

        VkSwapchainCreateInfoKHR createInfo
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface =  m_gpuWrapper.surface(),
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
            .preTransform = swapChainSupport.capabilities.currentTransform
        };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = to_u32(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        if (vkCreateSwapchainKHR(m_gpuWrapper. m_gpuWrapper(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
        {
            throw Exception::SwapChainError("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_gpuWrapper. m_gpuWrapper(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);

        vkGetSwapchainImagesKHR(m_gpuWrapper. m_gpuWrapper(), m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
     }

    template<bool ValidationEnabled>
    void
    SwapChain<ValidationEnabled>::createImageViews()
    {
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (Index_t i{ 0 }; i < m_swapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo viewInfo
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_swapChainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_swapChainImageFormat,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = 1,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1
            };

            if (vkCreateImageView(m_gpuWrapper. m_gpuWrapper(), &viewInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw Exception::SwapChainError("failed to create texture image view!");
            }
        }
     }

    template<bool ValidationEnabled>
    void
    SwapChain<ValidationEnabled>::createRenderPass()
    {
        VkAttachmentDescription depthAttachment
        {
            .format = findDepthFormat(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference depthAttachmentRef
        {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentDescription colorAttachment
        {
            .format = getSwapChainImageFormat(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference colorAttachmentRef
        {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass
        {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pDepthStencilAttachment = &depthAttachmentRef
        };

        VkSubpassDependency dependency
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
        };

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

        VkRenderPassCreateInfo renderPassInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = to_u32(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
        };

        if (vkCreateRenderPass(m_gpuWrapper. m_gpuWrapper(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        {
            throw Exception::SwapChainError("failed to create render pass!");
        }
    }

    template<bool ValidationEnabled>
    void
    SwapChain<ValidationEnabled>::createFramebuffers()
    {
        m_swapChainFramebuffers.resize(imageCount());

        for (Index_t i{ 0 }; i < imageCount(); ++i)
        {
            std::array<VkImageView, 2> attachments = {m_swapChainImageViews[i], m_depthImageViews[i]};

            VkExtent2D swapChainExtent = getSwapChainExtent();
            VkFramebufferCreateInfo framebufferInfo
            {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_renderPass,
                .attachmentCount = to_u32(attachments.size()),
                .pAttachments = attachments.data(),
                .width = swapChainExtent.width,
                .height = swapChainExtent.height,
                .layers = 1
            };

            if (vkCreateFramebuffer(m_gpuWrapper. m_gpuWrapper(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw Exception::SwapChainError("failed to create framebuffer!");
            }
        }
    }

    template<bool ValidationEnabled>
    void
    SwapChain<ValidationEnabled>::createDepthResources()
    {
        VkFormat depthFormat = findDepthFormat();
        VkExtent2D swapChainExtent = getSwapChainExtent();

        m_depthImages.resize(imageCount());
        m_depthImageMemories.resize(imageCount());
        m_depthImageViews.resize(imageCount());

        for (Index_t i{ 0 }; i < m_depthImages.size(); ++i)
        {
            VkImageCreateInfo imageInfo
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D,
                .extent.width = swapChainExtent.width,
                .extent.height = swapChainExtent.height,
                .extent.depth = 1,
                .mipLevels = 1,
                .arrayLayers = 1,
                .format = depthFormat,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .flags = 0,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE
            };

            m_gpuWrapper.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImages[i], m_depthImageMemories[i]);

            VkImageViewCreateInfo viewInfo
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_depthImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = depthFormat,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = 1,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1
            };

            if (vkCreateImageView(m_gpuWrapper. m_gpuWrapper(), &viewInfo, nullptr, &m_depthImageViews[i]) != VK_SUCCESS)
            {
                throw Exception::SwapChainError("failed to create texture image view!");
            }
        }
    }

    template<bool ValidationEnabled>
    void
    SwapChain<ValidationEnabled>::createSyncObjects()
    {
        m_imageAvailableSemaphores.resize(ImagesInFlight);
        m_renderFinishedSemaphores.resize(ImagesInFlight);
        m_inFlightFences.resize(ImagesInFlight);
        m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo
        {
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        VkFenceCreateInfo fenceInfo
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        for (Index_t i{ 0 }; i < ImagesInFlight; ++i)
        {
            if (vkCreateSemaphore(m_gpuWrapper.m_gpuWrapper(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS)
            {
                throw Exception::SwapChainError("failed to create synchronization objects for a frame!");
            }
            if (vkCreateSemaphore(m_gpuWrapper.m_gpuWrapper(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
            {
                throw Exception::SwapChainError("failed to create synchronization objects for a frame!");
            }
            if (vkCreateFence(m_gpuWrapper.m_gpuWrapper(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            {
                throw Exception::SwapChainError("failed to create synchronization objects for a frame!");
            }
        }
    }
    template<bool ValidationEnabled>
    VkSurfaceFormatKHR
    SwapChain<ValidationEnabled>::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    template<bool ValidationEnabled>
    VkPresentModeKHR
    SwapChain<ValidationEnabled>::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
         for (const auto &presentMode : availablePresentModes)
         {
             if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
             {
                 log::info("Present mode: Mailbox");

                 return presentMode;
             }
         }

        for (const auto &presentMode : availablePresentModes)
        {
            if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                log::info("Present mode: Immediate");

                return presentMode;
            }
        }

        log::info("Present mode: Immediate");

         return VK_PRESENT_MODE_FIFO_KHR;
     }

    template<bool ValidationEnabled>
    VkExtent2D
    SwapChain<ValidationEnabled>::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
         if (capabilities.currentExtent.width != Bad<u32>())
         {
             return capabilities.currentExtent;
         }
         else
         {
             VkExtent2D actualExtent = m_windowExtent;

             actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));

             actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

             return actualExtent;
         }
     }

    template<bool ValidationEnabled>
    VkFormat
    SwapChain<ValidationEnabled>::findDepthFormat()
    {
        return m_gpuWrapper.findSupportedFormat( {  VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                                                    VK_IMAGE_TILING_OPTIMAL,
                                                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}