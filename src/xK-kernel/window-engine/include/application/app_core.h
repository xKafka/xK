//
// Created by kafka on 2/3/2022.
//

#ifndef XGK_APPLICATION_H
#define XGK_APPLICATION_H

#include <window/win_main_window.h>
#include <xgk-vulkan/vk_core.h>
#include <utility/literal.h>

namespace xgk::application
{
    class Core
    {
#ifdef RELEASE
        static constexpr bool ValidationLayersEnable{ false };
#elif DEBUG
        static constexpr bool ValidationLayersEnable{ true };
#endif
        using MainWindow = win::MainWindow;

    public:
        Core(int argc, const char* argv[]);

        void run();

    private:
        std::shared_ptr<MainWindow> m_mainWindow;

        xgk::vulkan::Core<ValidationLayersEnable> m_vulkan;
    };
}

using Application = xgk::application::Core;

#endif //XGK_APPLICATION_H
