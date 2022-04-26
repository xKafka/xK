//
// Created by kafka on 4/22/2022.
//

#ifndef GRAPHICS_ENGINE_CONFIG_H
#define GRAPHICS_ENGINE_CONFIG_H

#include <array>
#include <utility/literal.h>

namespace xk::graphics_engine
{
    enum GraphicsApi
    {
        OpenGL,
        Vulkan,

        Count
    };
}

#endif //GRAPHICS_ENGINE_CONFIG_H
