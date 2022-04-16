//
// Created by kafka on 2/6/2022.
//
#include <fstream>
#include <filesystem>
#include <iostream>

#include <xgk-vulkan/vk_pipeline.h>
#include <xgk-vulkan/vk_exceptions.h>

namespace xgk::vulkan
{
    template<bool ValidationEnabled>
    Pipeline<ValidationEnabled>::Pipeline(const GpuWrapper<ValidationEnabled>& instance)
        : m_instance{ &instance }
        , m_vertShaderModule{ VK_NULL_HANDLE }
        , m_fragmentShaderModule{ VK_NULL_HANDLE }
        , m_pipeline{ VK_NULL_HANDLE }
    {}

    template<bool ValidationEnabled>
    Pipeline<ValidationEnabled>::~Pipeline()
    {
        vkDestroyShaderModule(m_instance->getLogicalDevice(), m_vertShaderModule, nullptr);
        vkDestroyShaderModule(m_instance->getLogicalDevice(), m_fragmentShaderModule, nullptr);

        vkDestroyPipeline(m_instance->getLogicalDevice(), m_pipeline, nullptr);
    }

    template<bool ValidationEnabled>
    void
    Pipeline<ValidationEnabled>::init(std::string_view vertFilePath, std::string_view fragFilePath, const PipelineConfigInfo& config)
    {
        createGraphicsPipeline(vertFilePath, fragFilePath, config);
    }

    template<bool ValidationEnabled>
    auto
    Pipeline<ValidationEnabled>::readFile(std::string_view filePath) -> std::vector<char>
    {
        std::ifstream file{ filePath.data(), std::ios::binary };

        if(!file.is_open())
        {
            throw Exception::PipelineError("failed to open file " + std::string{ filePath });
        }

        const u64 size{ std::filesystem::file_size(filePath) };

        std::vector<char> buffer(size);

        file.read(buffer.data(), size);

        file.close();

        return buffer;
    }

    template<bool ValidationEnabled>
    void
    Pipeline<ValidationEnabled>::createGraphicsPipeline(std::string_view vertFilePath, std::string_view fragFilePath, const PipelineConfigInfo& config)
    {
        if(config.pipelineLayout == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Cannot create graphics pipeline:: no pipelineLayout provided in config");
        }

        if(config.renderPass == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Cannot create graphics pipeline:: no renderPass provided in config");
        }

        const auto vertCode{ readFile(vertFilePath) };

        const auto fragCode{ readFile(fragFilePath) };

        m_vertShaderModule = createShaderModule(vertCode);

        m_fragmentShaderModule = createShaderModule(fragCode);

        static constexpr auto NumberOfShaders{ 2 }; //vertex + fragment shader

        static constexpr auto VertexShaderIndex{ 0 };

        static constexpr auto FragmentShaderIndex{ 1 };

       // auto& bindingDescriptions = pipelineCreateInfo.bindingDescriptions;
       // auto& attributeDescriptions = pipelineCreateInfo.attributeDescriptions;

        std::array<VkPipelineShaderStageCreateInfo, NumberOfShaders> shaderStages{};

        shaderStages.at(VertexShaderIndex).sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages.at(VertexShaderIndex).stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages.at(VertexShaderIndex).module = m_vertShaderModule;
        shaderStages.at(VertexShaderIndex).pName = "main";
        shaderStages.at(VertexShaderIndex).flags = 0;
        shaderStages.at(VertexShaderIndex).pNext = nullptr;
        shaderStages.at(VertexShaderIndex).pSpecializationInfo = nullptr;

        shaderStages.at(FragmentShaderIndex).sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages.at(FragmentShaderIndex).stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages.at(FragmentShaderIndex).module = m_fragmentShaderModule;
        shaderStages.at(FragmentShaderIndex).pName = "main";
        shaderStages.at(FragmentShaderIndex).flags = 0;
        shaderStages.at(FragmentShaderIndex).pNext = nullptr;
        shaderStages.at(FragmentShaderIndex).pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        //vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size());
        //vertexInputInfo.vertexBindingDescriptionCount = static_cast<u32>(bindingDescriptions.size());
        //vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        //vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        //pipelineCreateInfo.stageCount = shaderStages.size();
        //pipelineCreateInfo.pStages = shaderStages.data();
        //pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
        //pipelineCreateInfo.pInputAssemblyState = &config.inputAssemblyStateCreateInfo;
        //pipelineCreateInfo.pViewportState = &config.viewportInfo;
        //pipelineCreateInfo.pRasterizationState = &config.rasterizationInfo;
        //pipelineCreateInfo.pMultisampleState = &config.multisampleInfo;
        //pipelineCreateInfo.pColorBlendState = &config.colorBlendInfo;
        //pipelineCreateInfo.pDepthStencilState = &config.depthStencilInfo;
        //pipelineCreateInfo.pDynamicState = &config.dynamicStateInfo;
        //pipelineCreateInfo.pDynamicState = nullptr;

        pipelineCreateInfo.layout = config.pipelineLayout;
        pipelineCreateInfo.renderPass = config.renderPass;
        pipelineCreateInfo.subpass = config.subpass;

        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

        if(vkCreateGraphicsPipelines(m_instance->getLogicalDevice(), VK_NULL_HANDLE,1 , &pipelineCreateInfo, nullptr, &m_pipeline) != VK_SUCCESS)
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

        if(vkCreateShaderModule(m_instance->getLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {

        }

        return shaderModule;
    }

    template class Pipeline<true>;
    template class Pipeline<false>;
}
