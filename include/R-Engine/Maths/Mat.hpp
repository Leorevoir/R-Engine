#pragma once

#include <R-Engine/Maths/Concepts.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/R-EngineExport.hpp>

#include <cassert>
#include <initializer_list>

namespace r {

template<usize N, typename T>
    requires concepts::Vec<N, T>
struct R_ENGINE_API Mat {
    public:
        using value_type = T;
        static constexpr usize size = N;

        T data[N * N]{};

        constexpr Mat() = default;
        constexpr explicit Mat(const T &v);

        static constexpr Mat identity();

        constexpr Mat(std::initializer_list<T> il);
        constexpr T &operator()(const usize r, const usize c);
        constexpr const T &operator()(const usize r, const usize c) const;

        constexpr Mat transpose() const;
        constexpr Mat operator*(const Mat &other) const;

        template<typename VecT>
            requires concepts::IsSameAs<T, typename VecT::value_type>
        constexpr Vec<N, T> operator*(const VecT &v) const;

        constexpr bool operator==(const Mat &other) const;
};

/**
* Mat<N, T> instanciations
*/

template<typename T>
using Mat2 = Mat<2, T>;
template<typename T>
using Mat3 = Mat<3, T>;
template<typename T>
using Mat4 = Mat<4, T>;

using Mat2f = Mat2<f32>;
using Mat3f = Mat3<f32>;
using Mat4f = Mat4<f32>;

using Mat2d = Mat2<f64>;
using Mat3d = Mat3<f64>;
using Mat4d = Mat4<f64>;

using Mat2i = Mat2<i32>;
using Mat3i = Mat3<i32>;
using Mat4i = Mat4<i32>;

using Mat2u = Mat2<u32>;
using Mat3u = Mat3<u32>;
using Mat4u = Mat4<u32>;

/**
* helper functions for 4x4f
*/

/**
* @brief creates a translation matrix
* @param t translation vector
*/
constexpr inline R_ENGINE_API Mat4f translation(const r::Vec3f &t);

/**
* @brief creates a scaling matrix
* @param s scaling vector
*/
constexpr inline R_ENGINE_API Mat4f scale(const r::Vec3f &s);

/**
* @brief creates a rotation matrix around the X axis
* @param angle angle in radians
*/
inline R_ENGINE_API Mat4f rotation_x(const f32 angle);

/**
* @brief creates a rotation matrix around the Y axis
* @param angle angle in radians
*/
inline R_ENGINE_API Mat4f rotation_y(const f32 angle);

/**
* @brief creates a rotation matrix around the Z axis
* @param angle angle in radians
*/
inline R_ENGINE_API Mat4f rotation_z(const f32 angle);

#include "Inline/Mat.inl"

}// namespace r
