//
// Created by kafka on 2/6/2022.
//
#include <fstream>
#include <filesystem>
#include <iostream>

#include <xk-graphics-engine/xk-vulkan/vk_pipeline.h>
#include <xk-graphics-engine/xk-vulkan/vk_exceptions.h>
#include <xk-graphics-engine/xk-vulkan/vk_pipeline_config_info.h>

namespace xk::graphics_engine::vulkan 
{
    template<bool ValidationEnabled>
    Pipeline<ValidationEnabled>::Pipeline(std::weak_ptr<GpuWrapper<ValidationEnabled>> gpuWrapper)
        : m_gpuWrapper{ gpuWrapper }
        , m_pipeline{}
        , m_vertexShaderModule{}
        , m_fragmentShaderModule{}
    {}

    template<bool ValidationEnabled>
    Pipeline<ValidationEnabled>::~Pipeline()
    {
        auto instance = m_gpuWrapper.lock().get();

        vkDestroyShaderModule(instance->getLogicalDevice(), m_vertexShaderModule, nullptr);
        vkDestroyShaderModule(instance->getLogicalDevice(), m_fragmentShaderModule, nullptr);
        vkDestroyPipeline(instance->getLogicalDevice(), m_pipeline, nullptr);
    }

    template<bool ValidationEnabled>
    std::shared_ptr<Pipeline<ValidationEnabled>>
    Pipeline<ValidationEnabled>::createPipeline(std::weak_ptr<InstanceT> gpuWrapper)
    {
        return std::make_shared<Pipeline<ValidationEnabled>>(gpuWrapper);
    }

    template<bool ValidationEnabled>
    std::vector<char>
    Pipeline<ValidationEnabled>::readFile(std::string_view filePath)
    {
        std::ifstream file{ filePath.data(), std::ios::binary };

        if(!file.is_open())
        {
            throw Exception::PipelineError("failed to open file " + std::string{ filePath });
        }

        const u64 size{ std::filesystem::file_size(filePath) };

        std::vector<char> buffer(size);

        file.read(buffer.data(), static_cast<u32>(size));

        file.close();

        return buffer;
    }

    template<bool ValidationEnabled>
    void
    Pipeline<ValidationEnabled>::init()
    {
        createGraphicsPipeline();
    }

    template<bool ValidationEnabled>
    void
    Pipeline<ValidationEnabled>::createGraphicsPipeline()
    {
        auto createInfo = PipelineConfigInfo::defaultPipelineConfigInfo();

        if(!createInfo.pipelineLayout)
        {
            throw std::runtime_error("Cannot create graphics pipeline:: no pipelineLayout provided in config");
        }

        if(!createInfo.renderPass)
        {
            throw std::runtime_error("Cannot create graphics pipeline:: no renderPass provided in config");
        }

        const auto vertCode{ readFile(VertexShaderPath) };

        const auto fragCode{ readFile(FragmentShaderPath) };

        m_vertexShaderModule = createShaderModule(vertCode);

        m_fragmentShaderModule = createShaderModule(fragCode);

        static constexpr auto NumberOfShaders{ 2 }; //vertex + fragment shader

        static constexpr auto VertexShaderIndex{ 0 };

        static constexpr auto FragmentShaderIndex{ 1 };

        std::array<VkPipelineShaderStageCreateInfo, NumberOfShaders> shaderStages{};

        shaderStages[VertexShaderIndex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[VertexShaderIndex].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[VertexShaderIndex].module = m_vertexShaderModule;
        shaderStages[VertexShaderIndex].pName = "main";
        shaderStages[VertexShaderIndex].flags = 0;
        shaderStages[VertexShaderIndex].pNext = nullptr;
        shaderStages[VertexShaderIndex].pSpecializationInfo = nullptr;

        shaderStages[FragmentShaderIndex].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[FragmentShaderIndex].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[FragmentShaderIndex].module = m_fragmentShaderModule;
        shaderStages[FragmentShaderIndex].pName = "main";
        shaderStages[FragmentShaderIndex].flags = 0;
        shaderStages[FragmentShaderIndex].pNext = nullptr;
        shaderStages[FragmentShaderIndex].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<u32>(createInfo.bindingDescriptions.size()),
            .pVertexBindingDescriptions = createInfo.bindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<u32>(createInfo.attributeDescriptions.size()),
            .pVertexAttributeDescriptions = createInfo.attributeDescriptions.data()
        };

        VkGraphicsPipelineCreateInfo pipelineInfo
        {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &createInfo.inputAssemblyStateCreateInfo,
            .pViewportState = &createInfo.viewportStateCreateInfo,
            .pRasterizationState = &createInfo.rasterizationStateCreateInfo,
            .pMultisampleState = &createInfo.multisampleStateCreateInfo,
            .pDepthStencilState = &createInfo.depthStencilStateCreateInfo,
            .pColorBlendState = &createInfo.colorBlendStateCreateInfo,
            .pDynamicState = &createInfo.dynamicStateCreateInfo,
            .layout = createInfo.pipelineLayout,
            .renderPass = createInfo.renderPass,
            .subpass = createInfo.subPass,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        if (vkCreateGraphicsPipelines(m_gpuWrapper.lock()->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

    template<bool ValidationEnabled>
    VkShaderModule
    Pipeline<ValidationEnabled>::createShaderModule(const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const u32*>(code.data())
        };

        VkShaderModule shaderModule{};

        if (vkCreateShaderModule(m_gpuWrapper.lock()->getLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader");
        }

        return shaderModule;
    }

    template class Pipeline<true>;
    template class Pipeline<false>;
}
