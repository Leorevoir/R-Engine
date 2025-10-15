#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

#include "Startup/Startup.hpp"
#include "Update/Update.hpp"

// clang-format off

static inline const r::WindowPluginConfig G_WINDOW_CONFIG = {
    .size = {1280, 720},
    .title = "R-Engine - Lua Game Example",
    .cursor = r::WindowCursorState::Locked
};

int main(void)
{
    r::Application{}
        .add_plugins(
            r::DefaultPlugins{}.set(
                r::WindowPlugin{
                    r::WindowPluginConfig{G_WINDOW_CONFIG}
                }
            )
        )
        .add_systems<r::startup_load_player, r::startup_load_terrain, r::startup_load_inputs>(r::Schedule::STARTUP)
        .add_systems<r::update_inputs, r::update_player_position>(r::Schedule::UPDATE)
        .run();

    return 0;
}
