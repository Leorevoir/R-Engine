#include "Camera.hpp"
#include "Update.hpp"

// clang-format off

static constexpr f32 MOUSE_SENSITIVITY = 0.1f;

void r::update_player_position(
    const ecs::Res<UserInput> user_input,
    const ecs::Res<core::FrameTime> time,
    const ecs::Query<ecs::Mut<Transform3d>, ecs::Ref<Velocity>> query,
    const ecs::ResMut<Camera3d> camera
) noexcept
{
    auto &angles = r::get_camera_angles();
    r::update_camera_angles(angles, user_input.ptr->mouse_delta, MOUSE_SENSITIVITY);

    const auto vectors = r::calculate_camera_vectors(angles);

    for (auto [transform, velocity] : query) {
        transform.ptr->position += *velocity.ptr * time.ptr->delta_time;
        const Vec3f &player_pos = transform.ptr->position;

        camera.ptr->position = player_pos;
        camera.ptr->target = player_pos + vectors.forward;
        break;
    }
}
