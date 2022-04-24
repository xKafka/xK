//
// Created by kafka on 2/3/2022.
//

#ifndef XK_APPLICATION_H
#define XK_APPLICATION_H

#include <xk-graphics-engine/xk-engine/graphics_engine.h>
#include <window/win_main_window.h>
#include <utility/literal.h>

namespace xk::application 
{
    class Core
    {
#ifdef RELEASE
        static constexpr bool ValidationLayersEnable{ false };
#elif DEBUG
        static constexpr bool ValidationLayersEnable{ true };
#endif
        static constexpr graphics_engine::GraphicsApi UsedGraphicsApi{ graphics_engine::Vulkan };

        using GEngine = graphics_engine::GraphicsEngine<UsedGraphicsApi, ValidationLayersEnable>;

        using MainWindow = win::MainWindow<GEngine>;

    public:
        Core(int argc, const char* argv[]);

        void run();

    private:
        std::shared_ptr<GEngine> m_graphicsEngine;

        std::shared_ptr<MainWindow> m_mainWindow;
    };
}

using Application = xk::application::Core;

#endif //XK_APPLICATION_H
