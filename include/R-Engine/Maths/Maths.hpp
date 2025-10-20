#pragma once

#include <R-Engine/Types.hpp>

namespace r {

template<typename T>
constexpr T radians(const T degrees) noexcept;
static constexpr f32 R_PI = 3.14159265358979323846f;

}// namespace r

#include "Inline/Maths.inl"
