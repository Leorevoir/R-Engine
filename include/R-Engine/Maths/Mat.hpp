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

        /**
         * @brief constructs a matrix with all elements set to the same value
         * @param v value to initialize all matrix elements
         * 
         * Mat3f m(5.0f); ///<< creates:
         * 
         *     [5.0  5.0  5.0]
         *     [5.0  5.0  5.0]
         *     [5.0  5.0  5.0]
         */
        constexpr explicit Mat(const T &v);

        /**
         * @brief creates an identity matrix
         * @details identity matrix has 1s on the diagonal and 0s elsewhere
         * 
         * Mat3f::identity(); ///<< returns:
         * 
         *     [1.0  0.0  0.0]
         *     [0.0  1.0  0.0]
         *     [0.0  0.0  1.0]
         */
        static constexpr Mat identity();

        /**
         * @brief constructs a matrix from an initializer list (row-major order)
         * @param il initializer list containing N×N elements in row-major order
         * 
         * Mat2f m{1, 2,     ///<< creates:
         *         3, 4};    ///<< 
         *                   ///<<     [1.0  2.0]
         *                   ///<<     [3.0  4.0]
         */
        constexpr Mat(std::initializer_list<T> il);

        /**
         * @brief element access operator (row, column)
         * @param r row index (0-based)
         * @param c column index (0-based)
         * 
         * Mat3f m;
         * m(1, 2) = 5.0f; ///<< sets element at row 1, column 2
         * 
         *     [•   •   •]
         *     [•   •   5] ← row 1, col 2
         *     [•   •   •]
         */
        constexpr T &operator()(const usize r, const usize c);
        constexpr const T &operator()(const usize r, const usize c) const;

        /**
         * @brief returns the transpose of the matrix
         * @details swaps rows and columns: Aᵀ[i,j] = A[j,i]
         * 
         * original:        transpose:
         * [a  b  c]        [a  d  g]
         * [d  e  f]  --→   [b  e  h]
         * [g  h  i]        [c  f  i]
         */
        constexpr Mat transpose() const;

        /**
         * @brief matrix multiplication
         * @param other matrix to multiply with
         * @return result of matrix multiplication A × B
         * 
         * for each element C[i,j] = Σ A[i,k] × B[k,j]
         *                           k=0..N-1
         * 
         * example for 2x2:
         * 
         * A = [a b]   B = [e f]   A×B = [ae+bg  af+bh]
         *     [c d]       [g h]         [ce+dg  cf+dh]
         */
        constexpr Mat operator*(const Mat &other) const;

        /**
         * @brief matrix-vector multiplication
         * @param v vector to multiply with
         * @return transformed vector
         * 
         * ror each element v_out[i] = Σ M[i,j] × v[j]
         *                             j=0..N-1
         * 
         * [a b c]   [x]   [ax + by + cz]
         * [d e f] × [y] = [dx + ey + fz]
         * [g h i]   [z]   [gx + hy + iz]
         */
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

namespace mat {

/**
* @brief creates a translation matrix
* @param t translation vector
* 
* translation matrix for vector (x, y, z):
* 
*     [1  0  0  x]
*     [0  1  0  y]
*     [0  0  1  z]
*     [0  0  0  1]
* 
* v' = M × v = [v.x + x, v.y + y, v.z + z, 1]
*/
constexpr inline R_ENGINE_API Mat4f translation(const r::Vec3f &t);

/**
* @brief creates a scaling matrix
* @param s scaling vector
* 
* scaling matrix for factors (sx, sy, sz):
* 
*     [sx  0   0   0]
*     [0   sy  0   0]
*     [0   0   sz  0]
*     [0   0   0   1]
* 
* v' = M × v = [v.x × sx, v.y × sy, v.z × sz, 1]
*/
constexpr inline R_ENGINE_API Mat4f scale(const r::Vec3f &s);

/**
* @brief creates a rotation matrix around the X axis
* @param angle angle in radians
* 
* Rotation around X axis by angle θ:
* 
*     [1     0      0    0]
*     [0   cosθ  -sinθ   0]
*     [0   sinθ   cosθ   0]
*     [0     0      0    1]
* 
*        y
*        ↑    z
*        |   ╱
*        |  ╱ 
*        | ╱
*        ○─────→ x (rotation axis)
*/
inline R_ENGINE_API Mat4f rotation_x(const f32 angle);

/**
* @brief creates a rotation matrix around the Y axis
* @param angle angle in radians
* 
* Rotation around Y axis by angle θ:
* 
*     [ cosθ  0   sinθ   0]
*     [  0    1    0     0]
*     [-sinθ  0   cosθ   0]
*     [  0    0    0     1]
* 
*        z
*        ↑    x
*        |   ╱
*        |  ╱ 
*        | ╱
*        ○─────→ y (rotation axis)
*/
inline R_ENGINE_API Mat4f rotation_y(const f32 angle);

/**
* @brief creates a rotation matrix around the Z axis
* @param angle angle in radians
* 
* Rotation around Z axis by angle θ:
* 
*     [cosθ  -sinθ  0   0]
*     [sinθ   cosθ  0   0]
*     [ 0      0    1   0]
*     [ 0      0    0   1]
* 
*        x
*        ↑    y
*        |   ╱
*        |  ╱ 
*        | ╱
*        ○─────→ z (rotation axis)
*/
inline R_ENGINE_API Mat4f rotation_z(const f32 angle);

}// namespace mat

#include "Inline/Mat.inl"

}// namespace r
