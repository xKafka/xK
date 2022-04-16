//
// Created by kafka on 2/3/2022.
//

#ifndef XGK_XGK_WINDOWENGINE_INCLUDE_WINDOW_GK_MAIN_WINDOW_H_
#define XGK_XGK_WINDOWENGINE_INCLUDE_WINDOW_GK_MAIN_WINDOW_H_

#include "win_window_generic.h"

namespace xgk::win
{
    class MainWindow : public GenericWindow<true, true>
    {
        using Parent = GenericWindow<true, true>;

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
        MainWindow(u32 width, u32 height, std::string_view title)
            : Parent{ width, height, title }
        {}

        template<u32 Width, u32 Height>
        static constexpr auto createWindow(std::string_view title)
        {
            //do compile time things here

            return MainWindow{ Width, Height, title };
        }
    };
}

#endif //XGK_XGK_WINDOWENGINE_INCLUDE_WINDOW_GK_MAIN_WINDOW_H_
