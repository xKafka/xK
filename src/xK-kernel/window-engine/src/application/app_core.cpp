//
// Created by kafka on 2/3/2022.
//
#include <xgk-vulkan/vk_exceptions.h>

#include <application/app_core.h>

#include <utility/log.h>

namespace xgk::application
{
    Core::Core(const int argc, const char* argv[])
        : m_mainWindow{ std::make_shared<MainWindow>(MainWindow::createWindow<800, 600>("MainWindow")) }
        , m_vulkan{ vulkan::Core<ValidationLayersEnable>::createCore(m_mainWindow) }
    {}

    void Core::run()
    {
        m_mainWindow->show();

        while(!m_mainWindow->shouldClose())
        {
            m_mainWindow->update();
        }
    }
}