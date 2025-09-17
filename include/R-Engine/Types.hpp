#pragma once

#include <cmath>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using i8 = signed char;
using i16 = signed short;
using i32 = signed int;
using i64 = signed long long;

using f32 = float;
using f64 = double;

using usize = u64;
using isize = i64;

class NonCopyable
{
    protected:
        constexpr NonCopyable() = default;
        constexpr ~NonCopyable() = default;
        constexpr NonCopyable(const NonCopyable &) = delete;
        constexpr NonCopyable &operator=(const NonCopyable &) = delete;
        constexpr NonCopyable(NonCopyable &&) = delete;
        constexpr NonCopyable &operator=(NonCopyable &&) = delete;
};

#define R_ENGINE_EPSILON 1e-6f

namespace r {

namespace F32 {

/**
* @brief compare two floating point numbers for equality within a certain epsilon
* 
* @param a the first number
* @param b the second number
*/
static inline bool equal(const f32 a, const f32 b, const f32 epsilon = R_ENGINE_EPSILON) noexcept
{
    return std::fabs(a - b) <= epsilon * fmaxf(1.0f, fmaxf(std::fabs(a), std::fabs(b)));
}

}// namespace F32

template<typename T>
inline constexpr bool always_false = false;

}// namespace r
