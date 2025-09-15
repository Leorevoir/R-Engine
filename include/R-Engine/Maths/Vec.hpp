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

        constexpr Vec() = default;
        constexpr Vec(const T &value);

        constexpr Vec(const std::initializer_list<T> &il);

        template<typename... Args>
            requires(sizeof...(Args) == N) && concepts::AllConvertibleTo<T, Args...>
        constexpr Vec(Args &&...args);

        template<typename U>
            requires concepts::AllConvertibleTo<T, U>
        constexpr Vec(const Vec<N, U> &other);

        constexpr T dot(const Vec<N, T> &other) const;
        constexpr Vec<N, T> cross(const Vec<N, T> &other) const;
        constexpr Vec<N, T> normalize() const;
        constexpr T length() const;
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

/**
 * @brief operators
 */

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> operator+(const Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    Vec<N, T> result;

    for (usize i = 0; i < N; ++i) {
        result.data[i] = lhs.data[i] + rhs.data[i];
    }
    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> operator-(const Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    Vec<N, T> result;

    for (usize i = 0; i < N; ++i) {
        result.data[i] = lhs.data[i] - rhs.data[i];
    }
    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> operator*(const Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    Vec<N, T> result;

    for (usize i = 0; i < N; ++i) {
        result.data[i] = lhs.data[i] * rhs.data[i];
    }
    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> operator/(const Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    Vec<N, T> result;

    for (usize i = 0; i < N; ++i) {
        result.data[i] = lhs.data[i] / rhs.data[i];
    }
    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> operator*(const Vec<N, T> &lhs, const T &scalar)
{
    Vec<N, T> result;

    for (usize i = 0; i < N; ++i) {
        result.data[i] = lhs.data[i] * scalar;
    }
    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> operator*(const T &scalar, const Vec<N, T> &rhs)
{
    return rhs * scalar;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> operator/(const Vec<N, T> &lhs, const T &scalar)
{
    Vec<N, T> result;

    for (usize i = 0; i < N; ++i) {
        result.data[i] = lhs.data[i] / scalar;
    }
    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> &operator+=(Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    for (usize i = 0; i < N; ++i) {
        lhs.data[i] += rhs.data[i];
    }
    return lhs;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> &operator-=(Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    for (usize i = 0; i < N; ++i) {
        lhs.data[i] -= rhs.data[i];
    }
    return lhs;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> &operator*=(Vec<N, T> &lhs, const T &scalar)
{
    for (usize i = 0; i < N; ++i) {
        lhs.data[i] *= scalar;
    }
    return lhs;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> &operator/=(Vec<N, T> &lhs, const T &scalar)
{
    for (usize i = 0; i < N; ++i) {
        lhs.data[i] /= scalar;
    }
    return lhs;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr bool operator==(const Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    for (usize i = 0; i < N; ++i) {
        if (lhs.data[i] != rhs.data[i]) {
            return false;
        }
    }
    return true;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr bool operator!=(const Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    return !(lhs == rhs);
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> operator-(const Vec<N, T> &vec)
{
    Vec<N, T> result;

    for (usize i = 0; i < N; ++i) {
        result.data[i] = -vec.data[i];
    }
    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> &operator*=(Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    for (usize i = 0; i < N; ++i) {
        lhs.data[i] *= rhs.data[i];
    }
    return lhs;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> &operator/=(const Vec<N, T> &lhs, const Vec<N, T> &rhs)
{
    for (usize i = 0; i < N; ++i) {
        lhs.data[i] /= rhs.data[i];
    }
    return lhs;
}

/**
 * @brief implementation
 */

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T>::Vec(const T &value)
{
    for (usize i = 0; i < N; ++i) {
        this->data[i] = value;
    }
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T>::Vec(const std::initializer_list<T> &il)
{
    usize i = 0;

    for (const auto &v : il) {
        if (i < N) {
            this->data[i++] = v;
        }
    }
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
template<typename... Args>
    requires(sizeof...(Args) == N) && concepts::AllConvertibleTo<T, Args...>
constexpr Vec<N, T>::Vec(Args &&...args)
{
    const T temp[N] = {static_cast<T>(args)...};

    for (usize i = 0; i < N; ++i) {
        this->data[i] = temp[i];
    }
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
template<typename U>
    requires concepts::AllConvertibleTo<T, U>
constexpr Vec<N, T>::Vec(const Vec<N, U> &other)
{
    for (usize i = 0; i < N; ++i) {
        this->data[i] = static_cast<T>(other.data[i]);
    }
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr T Vec<N, T>::dot(const Vec<N, T> &other) const
{
    T result = static_cast<T>(0);

    for (usize i = 0; i < N; ++i) {
        result += this->data[i] * static_cast<T>(other.data[i]);
    }
    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> Vec<N, T>::cross(const Vec<N, T> &other) const
{
    if (N != 3) {
        return Vec<N, T>{0};
    }

    Vec<N, T> result;

    result.x = this->y * other.z - this->z * other.y;
    result.y = this->z * other.x - this->x * other.z;
    result.z = this->x * other.y - this->y * other.x;

    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Vec<N, T> Vec<N, T>::normalize() const
{
    const T length = std::sqrt(dot(*this));

    if (F32::equal(length, .0f)) {
        return *this;
    }

    Vec<N, T> result(*this);

    result /= length;
    return result;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr T Vec<N, T>::length() const
{
    return std::sqrt(dot(*this));
}

}// namespace r
