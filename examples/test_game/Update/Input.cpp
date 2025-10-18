#include "Camera.hpp"
#include "Update.hpp"

// clang-format off

static constexpr f32 G_PLAYER_SPEED = 10.f;
static constexpr r::Vec3f G_UP = {0.f, 1.f, 0.f};

void r::update_inputs(
    const ecs::Res<UserInput> user_input,
    const ecs::Res<InputMap> input_map,
    const ecs::Query<ecs::Mut<Velocity>, ecs::With<Controllable>> query) noexcept
{
    const auto *map = input_map.ptr;
    const auto &input = *user_input.ptr;
    const auto &vectors = r::calculate_camera_vectors(r::get_camera_angles());

    for (auto [velocity, _] : query) {
        Vec3f dir = {0.f, 0.f, 0.f};

        if (map->isActionPressed("MoveForward", input))
            dir += vectors.forward;
        if (map->isActionPressed("MoveBackward", input))
            dir -= vectors.forward;
        if (map->isActionPressed("MoveLeft", input))
            dir += vectors.right;
        if (map->isActionPressed("MoveRight", input))
            dir -= vectors.right;
        if (map->isActionPressed("MoveUp", input))
            dir += G_UP;
        if (map->isActionPressed("MoveDown", input))
            dir -= G_UP;

        const f32 length = dir.length();

        *velocity.ptr = (length > 0.f) ? (dir / length) * G_PLAYER_SPEED : dir;
    }
}
