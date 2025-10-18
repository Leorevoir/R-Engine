#pragma once

#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

namespace r {

void startup_load_terrain(ecs::Commands &commands, ecs::ResMut<Meshes> meshes) noexcept;
void startup_load_player(ecs::Commands &commands) noexcept;
void startup_load_inputs(ecs::ResMut<InputMap> input_map) noexcept;

}// namespace r
