#pragma once

#include <R-Engine/Maths/Vec.hpp>

namespace r {

/**
 * @brief 3D Transform structure
 * @details holds translation, rotation, and scale vectors for 3D transformations.
 */
struct R_ENGINE_API Transform3d {
    public:
        r::Vec3f position = {0.f, 0.f, 0.f};
        r::Vec3f rotation = {0.f, 0.f, 0.f};
        r::Vec3f scale = {1.f, 1.f, 1.f};
};

/**
 * @brief Computed global transform in world space.
 * @details Automatically computed by the transform propagation system.
 */
struct R_ENGINE_API GlobalTransform3d final : public Transform3d {
    public:
        static GlobalTransform3d from_local_and_parent(const Transform3d &local, const GlobalTransform3d &parent) noexcept;
};

}// namespace r
