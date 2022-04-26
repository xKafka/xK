//
// Created by kafka on 2/3/2022.
//

#ifndef XK_XK_WINDOWENGINE_INCLUDE_WINDOW_GK_WINDOW_H_
#define XK_XK_WINDOWENGINE_INCLUDE_WINDOW_GK_WINDOW_H_


/*external*/
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <fmt/format.h>

/*standard*/
#include <array>
#include <string>
#include <functional>
#include <limits>

/*custom*/
#include <utility/literal.h>
#include <utility/log.h>

#include <window/win_exceptions_generic.h>
#include <window/win_keyboard.h>

#include <xk-graphics-engine/xk-engine/graphics_engine.h>

namespace xk::win
{
    namespace limits
    {
        static constexpr u32 MaxWidth{ 800 };

        static constexpr u32 MaxHeight{ 600 };

        static constexpr u32 WindowTitleMaxChars{ 64 };
    }

    using WindowEngine = GLFWwindow;

    template<typename GraphicsApi, bool IsValidationEnabled>
    class GenericWindow
    {
        static constexpr bool IsGraphicsApiVulkanType() { return GraphicsApi::Type == graphics_engine::Vulkan; }

        static constexpr bool IsGraphicsApiOpenGlType() { return GraphicsApi::Type == graphics_engine::OpenGL; }

        static void frameBufferResizeCallback(WindowEngine* window, int width, int height);

        static void keyHandlerCallBack(WindowEngine* window, int key, int scancode, int action, int mods);

        static void windowCloseCallback(WindowEngine* window);

        virtual void keyPressedEvent(Key key, int scancode, int action, int mods) = 0;

        virtual void resizedEvent(u32 width, u32 height) = 0;

        virtual void closeEvent() = 0;

    protected:
        void init();

    public:
        GenericWindow(std::weak_ptr<GraphicsApi> graphicsEngine, u32 width, u32 height, std::string_view title);

        virtual ~GenericWindow() noexcept;

        void close();

        [[nodiscard]] inline bool shouldClose() const { return glfwWindowShouldClose(m_window); }

        void resize(u32 width, u32 height);

        void update();

        void show();

        void showFullWindow();

        [[nodiscard]] constexpr inline auto width() const { return m_width; }

        [[nodiscard]] constexpr inline auto height() const { return m_height; }

        [[nodiscard]] VkSurfaceKHR createVulkanWindowSurface(VkInstance vulkanInstance) const;

    private:
        std::weak_ptr<GraphicsApi> m_graphicsEngine;

        std::string m_title{};

        u32 m_width{}, m_height{};

        WindowEngine* m_window{};

        bool m_isShown{};
    };

    extern template class GenericWindow<graphics_engine::gl::Core<true>, true>;
    extern template class GenericWindow<graphics_engine::gl::Core<false>, true>;

    extern template class GenericWindow<graphics_engine::gl::Core<true>, false>;
    extern template class GenericWindow<graphics_engine::gl::Core<false>, false>;

    extern template class GenericWindow<graphics_engine::vulkan::Core<true>, true>;
    extern template class GenericWindow<graphics_engine::vulkan::Core<false>, true>;

    extern template class GenericWindow<graphics_engine::vulkan::Core<true>, false>;
    extern template class GenericWindow<graphics_engine::vulkan::Core<false>, false>;
}
#endif //XK_XK_WINDOWENGINE_INCLUDE_WINDOW_GK_WINDOW_H_
