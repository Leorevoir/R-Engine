#pragma once

namespace r {

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
