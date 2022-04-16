//
// Created by kafka on 2/6/2022.
//

#ifndef XGK_GK_VK_PIPELINE_H
#define XGK_GK_VK_PIPELINE_H

#include <utility/literal.h>
#include <xgk-vulkan/vk_gpu_wrapper.h>
#include <xgk-vulkan/vk_pipeline_config_info.h>

#include <vector>
#include <string>

namespace xgk::vulkan
{
    template<bool ValidationEnabled>
    class Pipeline
    {
        static std::vector<char> readFile(std::string_view filePath);

        VkShaderModule createShaderModule(const std::vector<char>& code);

        void createGraphicsPipeline(std::string_view vertFilePath, std::string_view fragFilePath, const PipelineConfigInfo& config);

    public:
        constexpr Pipeline(){}

        ~Pipeline();

        void init(std::string_view vertFilePath, std::string_view fragFilePath, const PipelineConfigInfo& config);

        Pipeline(const Pipeline&) = delete;

        Pipeline& operator=(const Pipeline&) = delete;

        Pipeline(const GpuWrapper<ValidationEnabled>& instance);

    private:
        const GpuWrapper<ValidationEnabled>* m_instance;

        VkPipeline m_pipeline;
        VkShaderModule m_vertShaderModule;
        VkShaderModule m_fragmentShaderModule;
    };

    extern template class Pipeline<true>;
    extern template class Pipeline<false>;
}

#endif //XGK_GK_VK_PIPELINE_H
