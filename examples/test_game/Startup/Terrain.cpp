#include <R-Engine/Core/Filepath.hpp>

#include "Startup.hpp"

static constexpr r::Vec3f G_TERRAIN_SCALE = {1.f, 1.f, 1.f};
static constexpr r::Vec3f G_TERRAIN_POSITION = {0.f, 0.f, 0.f};

void r::startup_load_terrain(ecs::Commands &commands, ecs::ResMut<Meshes> meshes) noexcept
{
    const std::string &gltf_path = path::get("examples/lua_game/assets/Models/terrain.glb");

    commands.spawn(
        Mesh3d{
            meshes.ptr->add(Mesh3d::Glb(gltf_path)),
        },
        Transform3d{.position = G_TERRAIN_POSITION, .scale = G_TERRAIN_SCALE});
}
