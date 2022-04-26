//
// Created by kafka on 2/18/2022.
//

#ifndef XK_VK_PIPELINE_CONFIG_INFO_H
#define XK_VK_PIPELINE_CONFIG_INFO_H

#include <vulkan/vulkan.hpp>
#include <utility/literal.h>

namespace xk::graphics_engine::vulkan
{
    struct PipelineConfigInfo
    {
        std::vector<VkVertexInputBindingDescription>      bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription>    attributeDescriptions{};
        std::vector<VkDynamicState>                       dynamicStateEnables{};

        VkPipelineInputAssemblyStateCreateInfo            inputAssemblyStateCreateInfo{};
        VkPipelineViewportStateCreateInfo                 viewportStateCreateInfo{};
        VkPipelineRasterizationStateCreateInfo            rasterizationStateCreateInfo{};
        VkPipelineMultisampleStateCreateInfo              multisampleStateCreateInfo{};
        VkPipelineColorBlendAttachmentState               colorBlendAttachmentState{};
        VkPipelineColorBlendStateCreateInfo               colorBlendStateCreateInfo{};
        VkPipelineDepthStencilStateCreateInfo             depthStencilStateCreateInfo{};
        VkPipelineDynamicStateCreateInfo                  dynamicStateCreateInfo{};
        VkPipelineLayout                                  pipelineLayout{};
        VkRenderPass                                      renderPass{};
        u32                                               subPass{};

        PipelineConfigInfo() = default;

        static PipelineConfigInfo defaultPipelineConfigInfo();
    };
}

#endif //XK_VK_PIPELINE_CONFIG_INFO_H
