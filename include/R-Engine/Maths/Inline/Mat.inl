#pragma once

#include <cassert>
#include <cmath>

/**
 * public Mat struct implementation
 */

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Mat<N, T>::Mat(const T &v)
{
    for (usize i = 0; i < N * N; ++i) {
        data[i] = v;
    }
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Mat<N, T>::Mat(std::initializer_list<T> il)
{
    usize i = 0;

    for (auto it = il.begin(); it != il.end() && i < N * N; ++it, ++i) {
        data[i] = *it;
    }
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr T &Mat<N, T>::operator()(usize r, usize c)
{
    assert(r < N && c < N && "Matrix indices out of bounds");
    return data[r * N + c];
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr const T &Mat<N, T>::operator()(usize r, usize c) const
{
    assert(r < N && c < N && "Matrix indices out of bounds");
    return data[r * N + c];
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Mat<N, T> Mat<N, T>::identity()
{
    Mat m{};

    for (usize i = 0; i < N; ++i) {
        m(i, i) = static_cast<T>(1);
    }
    return m;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Mat<N, T> Mat<N, T>::transpose() const
{
    Mat out{};

    for (usize r = 0; r < N; ++r) {
        for (usize c = 0; c < N; ++c) {
            out(c, r) = (*this)(r, c);
        }
    }
    return out;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr Mat<N, T> Mat<N, T>::operator*(const Mat &other) const
{
    Mat out{};

    for (usize r = 0; r < N; ++r) {
        for (usize c = 0; c < N; ++c) {
            T sum{};

            for (usize k = 0; k < N; ++k) {
                sum += (*this)(r, k) * other(k, c);
            }
            out(r, c) = sum;
        }
    }
    return out;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
template<typename VecT>
    requires concepts::IsSameAs<T, typename VecT::value_type>
constexpr Vec<N, T> Mat<N, T>::operator*(const VecT &v) const
{
    Vec<N, T> out{};

    for (usize r = 0; r < N; ++r) {
        T sum{};

        for (usize c = 0; c < N; ++c) {
            sum += (*this)(r, c) * v.data[c];
        }
        out.data[r] = sum;
    }
    return out;
}

template<usize N, typename T>
    requires concepts::Vec<N, T>
constexpr bool Mat<N, T>::operator==(const Mat &other) const
{
    if constexpr (concepts::IsFloatingPoint<T>) {
        for (usize i = 0; i < N * N; ++i) {
            if (!F32::equal(data[i], other.data[i])) {
                return false;
            }
        }
        return true;
    } else {
        for (usize i = 0; i < N * N; ++i) {
            if (data[i] != other.data[i]) {
                return false;
            }
        }
        return true;
    }
}

/**
 * public mat 4x4f helpers implementation
 */

constexpr inline Mat4f mat::translation(const Vec3f &t)
{
    Mat4f m = Mat4f::identity();
    m(0, 3) = t.data[0];
    m(1, 3) = t.data[1];
    m(2, 3) = t.data[2];
    return m;
}

constexpr inline Mat4f mat::scale(const Vec3f &s)
{
    Mat4f m{};

    m(0, 0) = s.data[0];
    m(1, 1) = s.data[1];
    m(2, 2) = s.data[2];
    m(3, 3) = 1.0f;
    return m;
}

inline Mat4f mat::rotation_x(const f32 angle)
{
    Mat4f m = Mat4f::identity();
    const f32 c = std::cos(angle), s = std::sin(angle);

    m(1, 1) = c;
    m(1, 2) = -s;
    m(2, 1) = s;
    m(2, 2) = c;
    return m;
}

inline Mat4f mat::rotation_y(const f32 angle)
{
    Mat4f m = Mat4f::identity();
    const f32 c = std::cos(angle), s = std::sin(angle);

    m(0, 0) = c;
    m(0, 2) = s;
    m(2, 0) = -s;
    m(2, 2) = c;
    return m;
}

inline Mat4f mat::rotation_z(const f32 angle)
{
    Mat4f m = Mat4f::identity();
    const f32 c = std::cos(angle), s = std::sin(angle);

    m(0, 0) = c;
    m(0, 1) = -s;
    m(1, 0) = s;
    m(1, 1) = c;
    return m;
}
