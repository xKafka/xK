//
// Created by kafka on 2/3/2022.
//
#include <xk-graphics-engine/xk-vulkan/vk_exceptions.h>

#include <application/app_core.h>

#include <utility/log.h>

namespace xk::application
{
    Core::Core([[maybe_unused]] const int argc, [[maybe_unused]] const char* argv[])
        : m_graphicsEngine{ GraphicsApi::createCore() }
        , m_mainWindow{ MainWindow::createMainWindow<700, 500>(m_graphicsEngine, "MainWindow") }
        , m_widgets{ }
    {
        m_graphicsEngine->setupForWindow(m_mainWindow);

        //m_graphicsEngine->init();
    }

    std::weak_ptr<Core::MainWindow>
    Core::getMainWindow()
    {
        return m_mainWindow;
    }

    void Core::run()
    {
        /*m_mainWindow->show();

        while(!m_mainWindow->shouldClose())
        {
            m_mainWindow->update();
        }*/
    }
}