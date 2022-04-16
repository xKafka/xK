//
// Created by kafka on 2/3/2022.
//

#ifndef XGK_XGK_WINDOWENGINE_INCLUDE_VULKAN_GK_VULKAN_H_
#define XGK_XGK_WINDOWENGINE_INCLUDE_VULKAN_GK_VULKAN_H_

#include <xgk-vulkan/vk_pipeline.h>
#include <xgk-vulkan/vk_gpu_wrapper.h>
#include <window/win_main_window.h>

namespace xgk::vulkan
{
    template<bool ValidationLayersEnable>
    class Core
    {
        using ParentWindow = xgk::win::MainWindow;

    public:
        explicit Core(std::weak_ptr<ParentWindow> parent);

        static constexpr Core createCore(std::weak_ptr<ParentWindow>&& parentWindow)
        {
            return Core{ parentWindow };
        }

        void init();

        [[nodiscard]] inline auto& pipeline() const { return m_pipeline; }

    private:
        std::weak_ptr<ParentWindow> m_parentWindow;

        GpuWrapper<ValidationLayersEnable> m_instance;

        Pipeline<ValidationLayersEnable> m_pipeline;
    };

    extern template class Core<true>;
    extern template class Core<false>;
}

#endif //XGK_XGK_WINDOWENGINE_INCLUDE_VULKAN_GK_VULKAN_H_
