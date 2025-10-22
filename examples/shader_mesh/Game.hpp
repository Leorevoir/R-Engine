#pragma once

#include <R-Engine/Maths/Vec.hpp>

namespace r {

struct Controllable {
};
struct Player {
};
struct Velocity final : public Vec3f {
    public:
        constexpr Velocity operator=(const r::Vec3f &other) noexcept
        {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
            return *this;
        }
};

}// namespace r
