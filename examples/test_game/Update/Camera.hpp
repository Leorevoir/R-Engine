#pragma once

#include <R-Engine/Maths/Vec.hpp>

namespace r {

struct CameraAngles {

        f32 yaw = 0.f;
        f32 pitch = 0.f;
};

struct CameraVectors {

        Vec3f forward;
        Vec3f right;
};

CameraAngles &get_camera_angles() noexcept;
CameraVectors calculate_camera_vectors(const CameraAngles &angles) noexcept;
void update_camera_angles(CameraAngles &angles, const Vec2f &mouse_delta, f32 sensitivity) noexcept;

}// namespace r
