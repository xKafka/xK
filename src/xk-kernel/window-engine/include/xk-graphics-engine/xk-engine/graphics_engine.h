//
// Created by kafka on 4/22/2022.
//

#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <memory>

#include <xk-graphics-engine/xk-engine/graphics_engine_config.h>
#include <xk-graphics-engine/xk-vulkan/vk_core.h>
#include <xk-graphics-engine/xk-opengl/gl_core.h>

namespace xk::win
{
    template<typename GraphicsApiHandler, bool IsValidationEnabled> class MainWindow;
}

namespace xk::graphics_engine
{
    //TODO Support for metal will be implemented later
    template<GraphicsApi GApi, bool ValidationLayersEnabled>
    struct GraphicsEngine
    {
        using Api = std::conditional_t<GApi == GraphicsApi::OpenGL, gl::Core<ValidationLayersEnabled>, vulkan::Core<ValidationLayersEnabled>>;
    };

/*
    template<GraphicsApi GApi, bool ValidationLayersEnabled>
    class GraphicsEngine
    {
        using Api = std::conditional<GApi == GraphicsApi::OpenGL, gl::Core<ValidationLayersEnabled>, vulkan::Core<ValidationLayersEnabled>>::type;

        using ParentWindow = win::MainWindow<Api, ValidationLayersEnabled>;

    public:
        using Type = Api::Type;

        GraphicsEngine();

        static std::shared_ptr<GraphicsEngine> createEngine();

        void setupForWindow(std::weak_ptr<ParentWindow> parentWindow);

    private:
        std::weak_ptr<ParentWindow> m_parentWindow;

        std::unique_ptr<Api> m_graphicsLibrary;
    };*/
}

#endif //GRAPHICS_ENGINE_H
