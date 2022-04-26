//
// Created by kafka on 4/26/2022.
//

#ifndef CAST_H
#define CAST_H

#include <type_traits>

#include "literal.h"

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept Integer = std::is_integral_v<T>;

template<Integer Type>
inline constexpr auto to_u32(Type number)
{
    return static_cast<u32>(number);
}

template<Integer Type>
inline constexpr auto to_f32(Type number)
{
    return static_cast<f32>(number);
}

template<Arithmetic T>
inline constexpr auto Bad()
{
    return std::numeric_limits<T>::max();
}

#endif //CAST_H
