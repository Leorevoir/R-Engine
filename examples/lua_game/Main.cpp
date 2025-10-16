#include "R-Engine/Application.hpp"
#include "R-Engine/ECS/Command.hpp"
#include "R-Engine/Plugins/DefaultPlugins.hpp"
#include "R-Engine/Plugins/LuaScriptingPlugin.hpp"
#include "R-Engine/Plugins/MeshPlugin.hpp"
#include "R-Engine/Plugins/WindowPlugin.hpp"

void setup_scene_system(r::ecs::Commands cmds, r::ecs::ResMut<r::Meshes> meshes)
{
    r::MeshHandle cube_handle = meshes.ptr->add(r::Mesh3d::Cube(1.0f));

    cmds.spawn(r::Mesh3d{cube_handle, r::Color{255, 0, 0, 255}}, r::Transform3d{{0.f, 0.5f, 0.f}}, r::Script{"Scripts/player.lua"});
}

static inline const r::WindowPluginConfig G_WINDOW_CONFIG = {.size = {1280, 720},
    .title = "R-Engine - Lua Game Example",
    .cursor = r::WindowCursorState::Locked};

int main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{G_WINDOW_CONFIG}}))
        .add_systems<setup_scene_system>(r::Schedule::STARTUP)
        .run();

    return 0;
}
