//
// Created by kafka on 2/3/2022.
//

#ifndef XGK_XGK_WINDOWENGINE_INCLUDE_WINDOW_GK_WINDOW_H_
#define XGK_XGK_WINDOWENGINE_INCLUDE_WINDOW_GK_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>
#include <string>
#include <thread>
#include <functional>
#include <limits>
#include <utility/literal.h>
#include <utility/log.h>
#include <window/win_exceptions_generic.h>
#include <window/win_keyboard.h>
#include <fmt/format.h>

namespace xgk::win
{
    namespace limits
    {
        static constexpr u32 MaxWidth{ 800 };

        static constexpr u32 MaxHeight{ 600 };

        static constexpr u32 WindowTitleMaxChars{ 4 };
    }

    using WindowEngine = GLFWwindow;

    template<bool IsMainWindow, bool IsValidationEnabled>
    class GenericWindow
    {
        static void frameBufferResizeCallback(WindowEngine* window, int width, int height);

        static void keyHandlerCallBack(WindowEngine* window, int key, int scancode, int action, int mods);

        static void windowCloseCallback(WindowEngine* window);

        virtual void keyPressedEvent(Key key, int scancode, int action, int mods) = 0;

        virtual void resizedEvent(u32 width, u32 height) = 0;

        virtual void closeEvent() = 0;

    protected:
        void init();

    public:
        GenericWindow(u32 width, u32 height, std::string_view title);

        template<u32 Width, u32 Height, u32 N>
        static consteval auto createGenericWindow(const char (&title)[N])
        {
            //TODO add window checks
            static_assert(Width > limits::MaxWidth, "Window can not have {Width} more than {limits::MaxWidth}");

            static_assert(Height > limits::MaxHeight, "Window can not have {Width} more than {limits::MaxWidth}");

            static_assert(N > limits::WindowTitleMaxChars, "Window name is too long" );

            return GenericWindow{ Width, Height, title };
        }

        ~GenericWindow() noexcept;

        void close();

        [[nodiscard]] inline bool shouldClose() const { return glfwWindowShouldClose(m_window); }

        void resize(u32 width, u32 height);

        void update();

        void show();

        void showFullWindow();

        [[nodiscard]] constexpr inline auto width() const { return m_width; }

        [[nodiscard]] constexpr inline auto height() const { return m_height; }

        [[nodiscard]] VkSurfaceKHR createVulkanWindowSurface(const VkInstance& vulkanInstance) const;

        std::vector<const char*> requiredWindowExtensions() const;

    private:
        std::string m_title{};

        WindowEngine* m_window{};

        u32 m_width{}, m_height{};

        bool m_isShown{};
    };

    extern template class GenericWindow<true, true>;
    extern template class GenericWindow<true, false>;
    extern template class GenericWindow<false, true>;
    extern template class GenericWindow<false, false>;
}
#endif //XGK_XGK_WINDOWENGINE_INCLUDE_WINDOW_GK_WINDOW_H_
