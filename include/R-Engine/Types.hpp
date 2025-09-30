#pragma once

#include <R-Engine/R-EngineExport.hpp>

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

class R_ENGINE_API NonCopyable
{
    protected:
        constexpr NonCopyable() = default;
        constexpr ~NonCopyable() = default;
        constexpr NonCopyable(const NonCopyable &) = delete;
        constexpr NonCopyable &operator=(const NonCopyable &) = delete;
        constexpr NonCopyable(NonCopyable &&) = delete;
        constexpr NonCopyable &operator=(NonCopyable &&) = delete;
};

static constexpr f32 R_ENGINE_EPSILON = 1e-6f;

namespace r {

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wpedantic"
#endif

struct R_ENGINE_API Color {
    public:
        union {
                struct {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
                        u8 r, g, b, a;
#else
                        u8 a, b, g, r;
#endif
                };
                u32 rgba;
        };

        constexpr inline Color(u8 red = 255, u8 green = 255, u8 blue = 255, u8 alpha = 255)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            : r(red), g(green), b(blue), a(alpha){}
#else
            : a(alpha), b(blue), g(green), r(red)
        {
            /* __ctor__ */
        }
#endif
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#elif defined(__clang__)
    #pragma clang diagnostic pop
#endif

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
