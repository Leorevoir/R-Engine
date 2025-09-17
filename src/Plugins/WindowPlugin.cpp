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

    u32 config_flags = 0;

    if ((config.ptr->settings & r::WindowPluginSettings::RESIZABLE) == r::WindowPluginSettings::RESIZABLE) {
        config_flags |= FLAG_WINDOW_RESIZABLE;
    }
    if (!((config.ptr->settings & r::WindowPluginSettings::DECORATED) == r::WindowPluginSettings::DECORATED)) {
        config_flags |= FLAG_WINDOW_UNDECORATED;
    }
    if ((config.ptr->settings & r::WindowPluginSettings::MAXIMIZED) == r::WindowPluginSettings::MAXIMIZED) {
        config_flags |= FLAG_WINDOW_MAXIMIZED;
    }
    if ((config.ptr->settings & r::WindowPluginSettings::ALWAYS_ON_TOP) == r::WindowPluginSettings::ALWAYS_ON_TOP) {
        config_flags |= FLAG_WINDOW_TOPMOST;
    }
    if ((config.ptr->settings & r::WindowPluginSettings::TRANSPARENT_FB) == r::WindowPluginSettings::TRANSPARENT_FB) {
        config_flags |= FLAG_WINDOW_TRANSPARENT;
    }

    SetConfigFlags(config_flags);
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
        .add_systems(Schedule::STARTUP, _init_window_system)
        .add_systems(Schedule::UPDATE, _update_window_system)
        .add_systems(Schedule::SHUTDOWN, _destroy_window_system);

    Logger::info("WindowPlugin built");
}
