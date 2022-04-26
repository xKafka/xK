//
// Created by kafka on 4/22/2022.
//

#include <xk-graphics-engine/xk-opengl/gl_core.h>

namespace xk::graphics_engine::gl
{
    template<bool ValidationLayersEnabled>
    void
    Core<ValidationLayersEnabled>::setupForWindow(std::weak_ptr<ParentWindow> parent)
    {

    }

    template<bool ValidationLayersEnabled>
    std::shared_ptr<Core<ValidationLayersEnabled>>
    Core<ValidationLayersEnabled>::createCore()
    {
        return std::make_shared<Core<ValidationLayersEnabled>>();
    }
}

