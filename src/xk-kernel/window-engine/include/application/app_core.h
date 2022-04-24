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

        using GraphicsApi = graphics_engine::GraphicsEngine<UsedGraphicsApi, ValidationLayersEnable>::Api;

        using Window = win::GenericWindow<GraphicsApi, ValidationLayersEnable>;

        using MainWindow = win::MainWindow<GraphicsApi, ValidationLayersEnable>;

        std::weak_ptr<MainWindow> getMainWindow();

    public:
        Core(int argc, const char* argv[]);

        void run();

    private:
        std::shared_ptr<GraphicsApi> m_graphicsEngine;

        std::shared_ptr<MainWindow> m_mainWindow;

        std::vector<std::shared_ptr<Window>> m_widgets;
    };
}

using Application = xk::application::Core;

#endif //XK_APPLICATION_H
