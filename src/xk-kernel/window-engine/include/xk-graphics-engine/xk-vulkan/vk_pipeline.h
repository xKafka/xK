//
// Created by kafka on 2/6/2022.
//

#ifndef XK_VK_PIPELINE_H
#define XK_VK_PIPELINE_H

#include <vector>
#include <string>
#include <memory>

#include <utility/literal.h>

#include "vk_gpu_wrapper.h"

namespace xk::graphics_engine::vulkan
{
    template<bool ValidationEnabled>
    class Pipeline
    {
        using InstanceT = GpuWrapper<ValidationEnabled>;

        static constexpr std::string_view VertexShaderPath{ "SOURCE_DIR R(Daatake)" };

        static constexpr std::string_view FragmentShaderPath{ "SOURCE_DIR R(Daatake)" };

        std::vector<char> readFile(std::string_view filePath);

        VkShaderModule createShaderModule(const std::vector<char>& code);

        void createGraphicsPipeline();

    public:
        explicit Pipeline(std::weak_ptr<InstanceT> gpuWrapper);

        ~Pipeline();

        static std::shared_ptr<Pipeline> createPipeline(std::weak_ptr<InstanceT> gpuWrapper);

        void init();

    private:
        std::weak_ptr<InstanceT> m_gpuWrapper;

        VkPipeline m_pipeline;
        VkShaderModule m_vertexShaderModule;
        VkShaderModule m_fragmentShaderModule;
    };

    extern template class Pipeline<true>;
    extern template class Pipeline<false>;
}

#endif //XK_VK_PIPELINE_H
