#pragma once

#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/R-EngineExport.hpp>

namespace r {

/* A small PI constant for math operations */
static constexpr f32 R_PI = 3.14159265358979323846f;

/**
 * @brief Represents a rotation in 3D space.
 * @details Quaternions are used to avoid the issues of gimbal lock that can occur with Euler angles.
 * They also make combining rotations (composition) computationally straightforward.
 */
struct R_ENGINE_API Quaternion {
    public:
        f32 x, y, z, w;

        /**
         * @brief Default constructor. Initializes to an identity quaternion (no rotation).
         */
        constexpr Quaternion() noexcept;

        /**
         * @brief Component-wise constructor.
         */
        constexpr Quaternion(f32 px, f32 py, f32 pz, f32 pw) noexcept;

        /**
         * @brief Creates a quaternion from Euler angles.
         * @param euler A Vec3f containing the Euler angles (in radians) for X, Y, and Z axes.
         * @return A quaternion representing the combined rotation.
         * @details The rotations are applied in the order: X-axis, then Y-axis, then Z-axis.
         * This corresponds to a ZYX extrinsic rotation sequence.
         */
        static Quaternion from_euler(const Vec3f &euler) noexcept;

        /**
         * @brief Converts this quaternion to Euler angles.
         * @return A Vec3f containing the Euler angles (in radians) for X, Y, and Z axes.
         * @details This conversion corresponds to a ZYX extrinsic rotation sequence and may
         * suffer from gimbal lock, where a degree of freedom is lost.
         */
        Vec3f to_euler() const noexcept;

        /**
         * @brief Combines this rotation with another.
         * @param rhs The other quaternion to multiply with.
         * @return A new quaternion representing the combined rotation.
         * @details The order of multiplication matters. `q1 * q2` means "first apply rotation q2, then q1".
         */
        constexpr Quaternion operator*(const Quaternion &rhs) const noexcept;

        /**
         * @brief Rotates a 3D vector by this quaternion.
         * @param v The vector to rotate.
         * @return The rotated vector.
         */
        Vec3f rotate(const Vec3f &v) const noexcept;

        /**
         * @brief Normalizes this quaternion to unit length.
         * @details This is important for ensuring the quaternion purely represents a rotation.
         */
        void normalize() noexcept;
};

}// namespace r

#include <R-Engine/Maths/Inline/Quaternion.inl>
