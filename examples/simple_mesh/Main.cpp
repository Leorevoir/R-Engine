#include "R-Engine/Application.hpp"
#include "R-Engine/Plugins/DefaultPlugins.hpp"
#include "R-Engine/Plugins/MeshPlugin.hpp"
#include "R-Engine/Plugins/WindowPlugin.hpp"
#include <R-Engine/Core/Filepath.hpp>

// clang-format off

static void startup_system(r::ecs::Commands &cmd, r::ecs::ResMut<r::Meshes> meshes)
{
    const std::string &glb_path = r::path::get("examples/simple_mesh/assets/robot.glb");

    cmd.spawn(
        r::Mesh3d{
            meshes.ptr->add(r::Mesh3d::Glb(glb_path)),
        },
        r::Transform3d{.translation = {0, -4, 0}, .scale = {1, 1, 1}}
    );
}

int main(void)
{
    r::Application {}
        .add_plugins(
            r::DefaultPlugins{}
            .set(r::WindowPlugin{
                r::WindowPluginConfig{
                    .size = {800, 600},
                    .title = "R-Engine Example",
                    .cursor = r::WindowCursorState::Visible
                }
            })
        )
        .add_systems(r::Schedule::STARTUP, startup_system)
        .run();
}
