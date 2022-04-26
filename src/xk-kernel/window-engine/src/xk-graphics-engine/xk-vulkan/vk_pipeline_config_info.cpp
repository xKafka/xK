//
// Created by kafka on 4/19/2022.
//

#include <xk-graphics-engine/xk-vulkan/vk_pipeline_config_info.h>

namespace xk::graphics_engine::vulkan 
{
    PipelineConfigInfo PipelineConfigInfo::defaultPipelineConfigInfo()
    {
        PipelineConfigInfo info{};

        info.inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        info.inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        info.inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        info.viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        info.viewportStateCreateInfo.viewportCount = 1;
        info.viewportStateCreateInfo.pViewports = nullptr;
        info.viewportStateCreateInfo.scissorCount = 1;
        info.viewportStateCreateInfo.pScissors = nullptr;

        info.rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        info.rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        info.rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        info.rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        info.rasterizationStateCreateInfo.lineWidth = 1.0f;
        info.rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        info.rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        info.rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        info.rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;  // Optional
        info.rasterizationStateCreateInfo.depthBiasClamp = 0.0f;           // Optional
        info.rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;     // Optional

        info.multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        info.multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        info.multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        info.multisampleStateCreateInfo.minSampleShading = 1.0f;           // Optional
        info.multisampleStateCreateInfo.pSampleMask = nullptr;             // Optional
        info.multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        info.multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;       // Optional

        info.colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                        VK_COLOR_COMPONENT_G_BIT |
                                                        VK_COLOR_COMPONENT_B_BIT |
                                                        VK_COLOR_COMPONENT_A_BIT;

        info.colorBlendAttachmentState.blendEnable = VK_FALSE;
        info.colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        info.colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        info.colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        info.colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        info.colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        info.colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        info.colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        info.colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        info.colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        info.colorBlendStateCreateInfo.attachmentCount = 1;
        info.colorBlendStateCreateInfo.pAttachments = &info.colorBlendAttachmentState;
        info.colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
        info.colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
        info.colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
        info.colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

        info.depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        info.depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        info.depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        info.depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        info.depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        info.depthStencilStateCreateInfo.minDepthBounds = 0.0f;  // Optional
        info.depthStencilStateCreateInfo.maxDepthBounds = 1.0f;  // Optional
        info.depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
        info.depthStencilStateCreateInfo.front = {};  // Optional
        info.depthStencilStateCreateInfo.back = {};   // Optional

        info.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        info.dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        info.dynamicStateCreateInfo.pDynamicStates = info.dynamicStateEnables.data();
        info.dynamicStateCreateInfo.dynamicStateCount = static_cast<u32>(info.dynamicStateEnables.size());
        info.dynamicStateCreateInfo.flags = 0;

        return info;
    }
}