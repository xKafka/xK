//
// Created by kafka on 2/3/2022.
//

#ifndef XK_XK_WINDOWENGINE_INCLUDE_WINDOW_GK_MAIN_WINDOW_H_
#define XK_XK_WINDOWENGINE_INCLUDE_WINDOW_GK_MAIN_WINDOW_H_

#include <window/win_window_generic.h>

namespace xk::win
{
    template<typename GraphicsApi, bool IsValidationEnabled>
    class MainWindow : public GenericWindow<GraphicsApi, IsValidationEnabled>
    {
        using Parent = GenericWindow<GraphicsApi, true>;

        void keyPressedEvent(Key key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, [[maybe_unused]] int mods) override
        {
            log::info("{}", toAscii(key));
        }

        void resizedEvent(u32 width, u32 height) override
        {
            log::info("Width {} Height {}", width, height);
        }

        void closeEvent() override
        {
            log::info("Closing window");
        }

    public:
        MainWindow(std::weak_ptr<GraphicsApi> graphicsApiHandler, u32 width, u32 height, std::string_view title)
            : Parent{ graphicsApiHandler, width, height, title }
        {}

        template<u32 Width, u32 Height, u32 N>
        static auto createMainWindow(std::weak_ptr<GraphicsApi> graphicsApiHandler, const char (&title)[N])
        {
            //TODO add window checks
            static_assert(Width < limits::MaxWidth, "Window can not have {Width} more than {limits::MaxWidth}");

            static_assert(Height < limits::MaxHeight, "Window can not have {Width} more than {limits::MaxWidth}");

            static_assert(N < limits::WindowTitleMaxChars, "Window name is too long" );

            return std::make_shared<MainWindow>(graphicsApiHandler, Width, Height, title);
        }
    };

    extern template class MainWindow<graphics_engine::gl::Core<true>, true>;
    extern template class MainWindow<graphics_engine::gl::Core<false>, true>;

    extern template class MainWindow<graphics_engine::vulkan::Core<true>, false>;
    extern template class MainWindow<graphics_engine::vulkan::Core<false>, false>;
}

#endif //XK_XK_WINDOWENGINE_INCLUDE_WINDOW_GK_MAIN_WINDOW_H_
