#pragma once

#include <R-Engine/Maths/Concepts.hpp>
#include <R-Engine/R-EngineExport.hpp>
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
    public:
        T data[N] = {0};
};

template<typename T>
struct VecData<2, T> {
    public:
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
                        T u, v;
                };
                struct {
                        T width, height;
                };
                T data[2] = {0};
        };
};

template<typename T>
struct VecData<3, T> {
    public:
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
    public:
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
 * @details N must be between 2 and 4 & T must be an arithmetic type
 *
 * vectors represent mathematical entities with magnitude and direction:
 * 
 *     2D: v = (x, y)       3D: v = (x, y, z)       4D: v = (x, y, z, w)
*          y                z                       (used for homogeneous coordinates)
 *         ↑                ↑  ╱
 *         |                | ╱
 *         |                |╱
 *         |                ○------→y
 *         ○------→x       ╱
 *                        x
 */
template<usize N, typename T>
    requires concepts::Vec<N, T>
struct R_ENGINE_API Vec : public details::VecData<N, T> {
    public:
        using value_type = T;
        static constexpr usize size = N;

        constexpr Vec() noexcept = default;

        /**
         * @brief constructs a vector with all components set to the same value
         * @param value value to initialize all vector components
         * 
         * Vec3f v(5.0f);
         * 
         *     z
         *     ↑   y
         *     |  ╱
         *     | ╱ 
         *     |╱ (5,5,5)
         *     ○─────→ x
         */
        constexpr Vec(const T &value) noexcept;

        /**
         * @brief constructs a vector from an initializer list
         * @param il initializer list containing N elements
         * 
         * Vec3f v{1.f, 2.f, 3.f};
         * 
         * components are assigned in order:
         *   - 2D: {x, y}
         *   - 3D: {x, y, z}  
         *   - 4D: {x, y, z, w}
         */
        constexpr Vec(const std::initializer_list<T> &il) noexcept;

        /**
         * @brief constructs a vector from individual component values
         * @param args N component values
         * 
         * Vec3f v(1.0f, 2.0f, 3.0f);
         * 
         * parameter order matches component order:
         *   - Vec2: (x, y)
         *   - Vec3: (x, y, z)
         *   - Vec4: (x, y, z, w)
         */
        template<typename... Args>
            requires(sizeof...(Args) == N) && concepts::AllConvertibleTo<T, Args...>
        constexpr Vec(Args &&...args) noexcept;

        /**
         * @brief copy constructor with type conversion
         * @param other vector to copy from (with potentially different but convertible type)
         * 
         * Vec3i v_int{1, 2, 3};
         * Vec3f v_float = static_cast<Vec3f>(v_int);
         * Vec3u v_uint = v_float; ///<< implicit conversion works too
         */
        template<typename U>
            requires concepts::AllConvertibleTo<T, U>
        constexpr Vec(const Vec<N, U> &other) noexcept;

        /**
         * @brief computes the dot product (scalar product) with another vector
         * @param other the other vector
         * 
         *   a · b = Σ aᵢ × bᵢ = aₓbₓ + aᵧbᵧ + a𝓏b𝓏 + ...
         *           i=0..N-1
         * 
         * geometric interpretation:
         *   a · b = |a| × |b| × cos(θ)
         * 
         * properties:
         *   - positive if vectors point in similar direction (θ < 90°)
         *   - zero if vectors are perpendicular (θ = 90°)
         *   - negative if vectors point in opposite directions (θ > 90°)
         */
        constexpr T dot(const Vec<N, T> &other) const noexcept;

        /**
         * @brief computes the cross product (vector product) with another vector (3D only)
         * @param other the other vector
         * 
         *   a × b = (aᵧb𝓏 - a𝓏bᵧ, a𝓏bₓ - aₓb𝓏, aₓbᵧ - aᵧbₓ)
         * 
         * geometric interpretation:
         *   - ragnitude: |a × b| = |a| × |b| × sin(θ)
         *   - direction: perpendicular to both a and b (right-hand rule)
         *
         * the resulting vector's magnitude equals the area of the parallelogram
         * spanned by the two input vectors.
         */
        constexpr Vec<N, T> cross(const Vec<N, T> &other) const noexcept;

        /**
         * @brief returns a normalized (unit) version of this vector
         * @details normalize = v / |v|  where |v| = sqrt(Σ v_i^2), i=0..N-1
         */
        constexpr Vec<N, T> normalize() const noexcept;

        /**
         * @brief returns the Euclidean length (magnitude) of this vector.
         * @details |v| = sqrt(Σ v_i^2), (i=0..N-1)
         */
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
using Vec3u = Vec3<u32>;

using Vec4f = Vec4<f32>;
using Vec4d = Vec4<f64>;
using Vec4i = Vec4<i32>;
using Vec4u = Vec4<u32>;

}// namespace r

#include "Inline/Vec.inl"
