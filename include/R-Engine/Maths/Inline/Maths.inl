#pragma once

#include <limits>

template<typename T>
constexpr T r::radians(const T degrees) noexcept
{
    static_assert(std::numeric_limits<T>::is_iec559, "'radians' only accept floating-point input");

    return degrees * static_cast<T>(0.01745329251994329576923690768489);
}
