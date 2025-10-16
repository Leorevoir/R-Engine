#pragma once

#include <R-Engine/Maths/Vec.hpp>

struct Player final {
};
struct Controllable final {
};
struct Velocity final : r::Vec3f {
    public:
        constexpr Velocity operator=(const r::Vec3f &other) noexcept
        {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
            return *this;
        }
};
