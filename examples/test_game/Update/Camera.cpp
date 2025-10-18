#include "Camera.hpp"

#include <R-Engine/Maths/Maths.hpp>

static constexpr f32 MIN_PITCH = -89.f;
static constexpr f32 MAX_PITCH = 89.f;

static r::CameraAngles g_camera_angles = {0.f, 0.f};

r::CameraAngles &r::get_camera_angles() noexcept
{
    return g_camera_angles;
}

void r::update_camera_angles(CameraAngles &angles, const Vec2f &mouse_delta, f32 sensitivity) noexcept
{
    angles.yaw -= mouse_delta.x * sensitivity;
    angles.pitch -= mouse_delta.y * sensitivity;

    if (angles.pitch > MAX_PITCH) {
        angles.pitch = MAX_PITCH;
    }
    if (angles.pitch < MIN_PITCH) {
        angles.pitch = MIN_PITCH;
    }
}

r::CameraVectors r::calculate_camera_vectors(const CameraAngles &angles) noexcept
{
    const f32 yaw_rad = r::radians(angles.yaw);
    const f32 pitch_rad = r::radians(angles.pitch);

    const f32 cos_pitch = std::cos(pitch_rad);
    const f32 sin_pitch = std::sin(pitch_rad);
    const f32 cos_yaw = std::cos(yaw_rad);
    const f32 sin_yaw = std::sin(yaw_rad);

    return {.forward = {sin_yaw * cos_pitch, sin_pitch, cos_yaw * cos_pitch}, .right = {cos_yaw, 0.f, -sin_yaw}};
}
