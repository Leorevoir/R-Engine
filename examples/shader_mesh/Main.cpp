#include "Startup/Startup.hpp"
#include "Update/Update.hpp"

#include <R-Engine/Application.hpp>
#include <R-Engine/Components/Material3d.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

// clang-format off

/**
 * @brief This system updates the 'globalTime' uniform for any material that has it.
 * @details This is necessary for time-based shader effects, like the pulsating
 * color on the planet in this example. It runs every frame during the UPDATE schedule.
 */
static void update_planet_shader_time(
    r::ecs::Res<r::core::FrameTime> time,
    r::ecs::Query<r::ecs::Mut<r::Material3d>> query
) noexcept
{
    for (auto [material] : query) {
        /* Update the "globalTime" uniform with the current total elapsed time. */
        material.ptr->set_uniform("globalTime", time.ptr->global_time);
    }
}

i32 main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{
            .size = {1280, 720},
            .title = "R-Engine - Shader Example",
            .cursor = r::WindowCursorState::Locked
        }}))

        .add_systems<
            r::startup_system_create_player,
            r::startup_system_create_planet,
            r::startup_system_create_inputs
        >(r::Schedule::STARTUP)

        .add_systems<
            r::update_inputs,
            r::update_player_position,
            update_planet_shader_time
        >(r::Schedule::UPDATE)

        .run();
}
