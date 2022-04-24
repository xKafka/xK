//
// Created by kafka on 2/21/2022.
//

#ifndef XK_WIN_EXCEPTIONS_H
#define XK_WIN_EXCEPTIONS_H

#include <stdexcept>
#include <type_traits>
#include <concepts>
#include <fmt/core.h>

namespace xk::win
{
    struct Exception : public std::runtime_error
    {
        using Parent = std::runtime_error;

        explicit Exception(std::string_view message)
            : Parent{ message.data() }
        {}

        static Exception WindowError(const auto& message)
        {
            return Exception{ fmt::format("WindowError::{}", message) };
        }

        static Exception VulkanSurfaceError(const auto& message)
        {
            return Exception{ fmt::format("VulkanSurfaceError::{}", message) };
        }
    };
}

#endif //XK_WIN_EXCEPTIONS_H
