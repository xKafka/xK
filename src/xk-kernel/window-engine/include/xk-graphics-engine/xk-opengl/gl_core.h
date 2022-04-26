//
// Created by kafka on 4/22/2022.
//

#ifndef GL_CORE_H
#define GL_CORE_H

#include <memory>

#include <xk-graphics-engine/xk-engine/graphics_engine_config.h>

namespace xk::win
{
    template<typename GraphicsApiHandler, bool IsValidationEnabled> class MainWindow;
}

namespace xk::graphics_engine::gl
{
    template<bool ValidationLayersEnabled>
    class Core
    {
        using ParentWindow = win::MainWindow<Core<ValidationLayersEnabled>, ValidationLayersEnabled>;

    public:
        static constexpr auto Type{ xk::graphics_engine::GraphicsApi::OpenGL };

        Core();

        void setupForWindow(std::weak_ptr<ParentWindow> parent);

        static std::shared_ptr<Core> createCore();
    };
}

#endif //GL_CORE_H
