//
// Created by kafka on 2/3/2022.
//

#include <xk-graphics-engine/xk-vulkan/vk_core.h>
#include <window/win_main_window.h>

namespace xk::graphics_engine::vulkan
{
    template<bool ValidationLayersEnabled>
    Core<ValidationLayersEnabled>::Core()
            : m_parentWindow{ }
            , m_instance{ InstanceT::createGpuWrapper() }
            , m_pipeline{ PipelineT::createPipeline(m_instance) }
    {
    }

    template<bool ValidationLayersEnabled>
    void
    Core<ValidationLayersEnabled>::setupForWindow(std::weak_ptr<ParentWindow> parent)
    {
        m_parentWindow = parent;

        m_instance->setupForWindow(parent);
        m_pipeline->init();
    }

    template<bool ValidationLayersEnabled>
    std::shared_ptr<Core<ValidationLayersEnabled>>
    Core<ValidationLayersEnabled>::createCore()
    {
        return std::make_shared<Core<ValidationLayersEnabled>>();
    }

    template class Core<true>;
    template class Core<false>;
}