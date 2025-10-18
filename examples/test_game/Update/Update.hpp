#pragma once

#include <R-Engine/Components/Transform3d.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>

#include "../Game.hpp"

// clang-format off

namespace r {

void update_inputs(
    const ecs::Res<UserInput> user_input,
    const ecs::Res<InputMap> input_map,
    const ecs::Query<ecs::Mut<Velocity>, ecs::With<Controllable>> query
) noexcept;

void update_player_position(
    const ecs::Res<UserInput> user_input,
    const ecs::Res<core::FrameTime> time,
    const ecs::Query<ecs::Mut<Transform3d>, ecs::Ref<Velocity>> query,
    const ecs::ResMut<Camera3d> camera
) noexcept;

// clang-format on

}// namespace r
