#pragma once

#include <R-Engine/Maths/Concepts.hpp>
#include <R-Engine/Types.hpp>

#include <initializer_list>

namespace r {

namespace details {

/**
 * NOTE: disable pedantic warnings for GCC/Clang to allow anonymous structs
 */

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wpedantic"
#endif

template<usize N, typename T>
struct VecData {
        T data[N] = {0};
};

template<typename T>
struct VecData<2, T> {
        union {
                struct {
                        T x, y;
                };
                struct {
                        T r, g;
                };
                struct {
                        T s, t;
                };
                struct {
                        T width, height;
                };
                T data[2] = {0};
        };
};

template<typename T>
struct VecData<3, T> {
        union {
                struct {
                        T x, y, z;
                };
                struct {
                        T r, g, b;
                };
                struct {
                        T s, t, p;
                };
                T data[3] = {0};
        };
};

template<typename T>
struct VecData<4, T> {
        union {
                struct {
                        T x, y, z, w;
                };
                struct {
                        T r, g, b, a;
                };
                struct {
                        T s, t, p, q;
                };
                T data[4] = {0};
        };
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#elif defined(__clang__)
    #pragma clang diagnostic pop
#endif

}// namespace details

/**
 * @brief mathematical vector of size N and type T
 * @info N must be between 2 and 4 & T must be an arithmetic type
 */
template<usize N, typename T>
    requires concepts::Vec<N, T>
struct Vec : public details::VecData<N, T> {
        using value_type = T;
        static constexpr usize size = N;

        constexpr Vec() noexcept = default;
        constexpr Vec(const T &value) noexcept;

        constexpr Vec(const std::initializer_list<T> &il) noexcept;

        template<typename... Args>
            requires(sizeof...(Args) == N) && concepts::AllConvertibleTo<T, Args...>
        constexpr Vec(Args &&...args) noexcept;

        template<typename U>
            requires concepts::AllConvertibleTo<T, U>
        constexpr Vec(const Vec<N, U> &other) noexcept;

        constexpr T dot(const Vec<N, T> &other) const noexcept;
        constexpr Vec<N, T> cross(const Vec<N, T> &other) const noexcept;
        constexpr Vec<N, T> normalize() const noexcept;
        constexpr T length() const noexcept;
};

template<typename T>
using Vec2 = Vec<2, T>;
template<typename T>
using Vec3 = Vec<3, T>;
template<typename T>
using Vec4 = Vec<4, T>;

using Vec2f = Vec2<f32>;
using Vec2d = Vec2<f64>;
using Vec2i = Vec2<i32>;
using Vec2u = Vec2<u32>;

using Vec3f = Vec3<f32>;
using Vec3d = Vec3<f64>;
using Vec3i = Vec3<i32>;

using Vec4f = Vec4<f32>;
using Vec4d = Vec4<f64>;
using Vec4i = Vec4<i32>;
using Vec4u = Vec4<u32>;

}// namespace r

#include "Inline/Vec.inl"
