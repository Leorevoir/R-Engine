#include <R-Engine/Plugins/WindowPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <R-Engine/Core/Backend.hpp>

/**
 * helpers
 */

static void _init_window_system(r::ecs::Res<r::WindowPluginConfig> config)
{
    const r::Vec2i size = static_cast<r::Vec2i>(config.ptr->size);
    const std::string &title = config.ptr->title;
    const i32 fps = static_cast<i32>(config.ptr->frame_per_second);

    InitWindow(size.width, size.height, title.c_str());
    SetTargetFPS(fps);
}

static void _update_window_system()
{
    if (WindowShouldClose()) {
        r::Application::quit = true;
    }
}

static void _destroy_window_system()
{
    CloseWindow();
}

/**
 * public
*/

r::WindowPlugin::WindowPlugin(const WindowPluginConfig &config) : _config(config)
{
    /* __ctor__ */
}

// clang-format off
void r::WindowPlugin::build(r::Application &app)
{
    app
        .insert_resource(_config)
        .add_system(Schedule::STARTUP, _init_window_system)
        .add_system(Schedule::UPDATE, _update_window_system)
        .add_system(Schedule::SHUTDOWN, _destroy_window_system);

    Logger::info("WindowPlugin built");
}
