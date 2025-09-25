#pragma once

#include <R-Engine/Maths/Mat.hpp>

namespace r {

/**
* @brief represents a 3D transformation with position, rotation (in radians), and scale
* @details
* - position: translation vector
* - rotation: Euler angles in radians (pitch, yaw, roll)
* - scale: scaling factors along each axis
*/
struct R_ENGINE_API Transform {
    public:
        Transform() = default;

        /**
        * @brief creates an identity transform (no translation, no rotation, unit scale)
        */
        static Transform Identity() noexcept;

        /**
        * @brief converts the transform to a 4x4 transformation matrix
        */
        Mat4f to_matrix() const noexcept;

        Vec3f position{0.f, 0.f, 0.f};
        Vec3f rotation{0.f, 0.f, 0.f};
        Vec3f scale{1.f, 1.f, 1.f};

    private:
};

}// namespace r
