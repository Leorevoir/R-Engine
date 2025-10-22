#pragma once

#include <R-Engine/Components/Shader.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

// clang-format off

namespace r {

void startup_system_create_planet(
    ecs::Commands &cmd,
    ecs::ResMut<Meshes> meshes,
    ecs::ResMut<Shaders> shaders,
    ecs::Res<core::FrameTime> frame_time 
) noexcept;
void startup_system_create_player(ecs::Commands &commands) noexcept;
void startup_system_create_inputs(ecs::ResMut<InputMap> input_map) noexcept;


}// namespace r
