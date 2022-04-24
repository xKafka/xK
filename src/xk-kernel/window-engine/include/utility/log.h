//
// Created by kafka on 2/6/2022.
//

#ifndef XK_LOG_H
#define XK_LOG_H

#include <spdlog/spdlog.h>

namespace xk::log
{
    template<typename... Args>
    using format_string_t = fmt::format_string<Args...>;

    template<typename... Args>
    void info(format_string_t<Args...> fmt, Args &&... args)
    {
        spdlog::log(spdlog::level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void debug(format_string_t<Args...> fmt, Args &&... args)
    {
        spdlog::log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void warning(format_string_t<Args...> fmt, Args &&... args)
    {
        spdlog::log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void error(format_string_t<Args...> fmt, Args &&... args)
    {
        spdlog::log(spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void critical(format_string_t<Args...> fmt, Args &&... args)
    {
        spdlog::log(spdlog::level::critical, fmt, std::forward<Args>(args)...);
    }
}

#endif //XK_LOG_H
