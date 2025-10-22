#include "Startup/Startup.hpp"
#include "Update/Update.hpp"

#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

// clang-format off

static inline const r::WindowPluginConfig G_WINDOW_CONFIG
{
    .size = {1280, 720},
    .title = "R-Engine - Lua Game Example",
    .cursor = r::WindowCursorState::Locked
};

i32 main(void)
{
    r::Application{}
        .add_plugins(
            r::DefaultPlugins{}.set(
                r::WindowPlugin{
                    r::WindowPluginConfig{G_WINDOW_CONFIG}
                }
            )
        )

        .add_systems<
            r::startup_system_create_player,
            r::startup_system_create_planet,
            r::startup_system_create_inputs
        >(r::Schedule::STARTUP)

        .add_systems<
            r::update_inputs,
            r::update_player_position
        >(r::Schedule::UPDATE)

        .run();
}
