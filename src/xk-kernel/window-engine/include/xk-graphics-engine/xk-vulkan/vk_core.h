//
// Created by kafka on 2/3/2022.
//

#ifndef XK_VK_CORE
#define XK_VK_CORE

#include <memory>
#include <xk-graphics-engine/xk-engine/graphics_engine_config.h>
#include <xk-graphics-engine/xk-vulkan/vk_gpu_wrapper.h>
#include <xk-graphics-engine/xk-vulkan/vk_pipeline.h>

namespace xk::win
{
    template<typename GraphicsApi, bool IsValidationEnabled> class MainWindow;
}

namespace xk::graphics_engine::vulkan
{
    template<bool ValidationLayersEnabled>
    class Core
    {
        using ParentWindow = win::MainWindow<Core<ValidationLayersEnabled>, ValidationLayersEnabled>;

        using PipelineT = Pipeline<ValidationLayersEnabled>;
        using InstanceT = GpuWrapper<ValidationLayersEnabled>;

    public:
        static constexpr auto Type{ xk::graphics_engine::GraphicsApi::Vulkan };

        explicit Core();

        std::shared_ptr<Core> createCore();

        void setupForWindow(std::weak_ptr<ParentWindow> parent);

        [[nodiscard]] inline auto& pipeline() const { return m_pipeline; }

    private:
        std::weak_ptr<ParentWindow> m_parentWindow;

        std::shared_ptr<InstanceT> m_instance;

        std::shared_ptr<PipelineT> m_pipeline;
    };

    extern template class Core<true>;
    extern template class Core<false>;
}

#endif //XK_VK_CORE
