//
// Created by kafka on 2/3/2022.
//

#include <xgk-vulkan/vk_core.h>

namespace xgk::vulkan
{
    template<bool ValidationLayersEnabled>
    Core<ValidationLayersEnabled>::Core(std::weak_ptr<ParentWindow> parent)
            : m_parentWindow{ parent }
            , m_instance{ parent }
            , m_pipeline{ m_instance }
    {
        auto sharedParent = m_parentWindow.lock();

        auto defaultPipelineInfo = PipelineConfigInfo::defaultPipelineConfigInfo(sharedParent->width(), sharedParent->height());

        init();
    }

    template<bool ValidationLayersEnabled>
    void
    Core<ValidationLayersEnabled>::init()
    {
        auto sharedParent = m_parentWindow.lock();

        m_instance.init();

        const auto defaultPipelineInfo = PipelineConfigInfo::defaultPipelineConfigInfo(sharedParent->width(), sharedParent->height());

        m_pipeline.init(SOURCE_DIR R"(/include/evolution-vulkan/shaders/simple_shader.vert.spv)",
                        SOURCE_DIR R"(/include/evolution-vulkan/shaders/simple_shader.frag.spv)",
                        defaultPipelineInfo);


    }

    template class Core<true>;
    template class Core<false>;
}