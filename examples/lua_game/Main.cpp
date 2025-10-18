#include "R-Engine/Application.hpp"
#include "R-Engine/Core/Filepath.hpp"
#include "R-Engine/ECS/Command.hpp"
#include "R-Engine/Plugins/DefaultPlugins.hpp"
#include "R-Engine/Plugins/LuaScriptingPlugin.hpp"
#include "R-Engine/Plugins/MeshPlugin.hpp"
#include "R-Engine/Plugins/WindowPlugin.hpp"

// clang-format off

void startup_system(r::ecs::Commands cmds, r::ecs::ResMut<r::Meshes> meshes)
{
    const std::string script_path = r::path::get("examples/lua_game/Scripts/script.lua");
    const r::MeshHandle cube_handle = meshes.ptr->add(
        r::Mesh3d::Cube(1.0f)
    );

    cmds.spawn(
        r::Mesh3d{
            cube_handle,
            r::Color{255, 0, 0, 255}
        },
        r::Transform3d{{0.f, 0.5f, 0.f}},
        r::Script{script_path}
    );
}

static inline const r::WindowPluginConfig G_WINDOW_CONFIG =
{
    .size = {1280, 720},
    .title = "R-Engine - Lua Game Example",
    .cursor = r::WindowCursorState::Visible
};

int main(void)
{
    r::Application{}
        .add_plugins(
            r::DefaultPlugins{}.set(
                r::WindowPlugin{
                    r::WindowPluginConfig{G_WINDOW_CONFIG}}
            )
        )
        .add_systems<startup_system>(r::Schedule::STARTUP)
        .run();

    return 0;
}
