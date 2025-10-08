#include <R-Engine/Plugins/WindowPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <R-Engine/Core/Backend.hpp>

/**
 * helpers
 */

static void _init_window_system(r::ecs::Res<r::WindowPluginConfig> config) noexcept
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

    switch (config.ptr->cursor) {
        case r::WindowCursorState::Visible:
            ShowCursor();
            break;
        case r::WindowCursorState::Hidden:
            HideCursor();
            break;
        case r::WindowCursorState::Locked:
            DisableCursor();
            break;
        default:
            ShowCursor();
            break;
    }
}

static void _update_cursor_system(r::ecs::Res<r::Cursor> cursor) noexcept
{
    switch (cursor.ptr->state) {
        case r::WindowCursorState::Visible:
            ShowCursor();
            break;
        case r::WindowCursorState::Hidden:
            HideCursor();
            break;
        case r::WindowCursorState::Locked:
            DisableCursor();
            break;
        default:
            ShowCursor();
            break;
    }
}

static void _update_window_system() noexcept
{
    if (WindowShouldClose()) {
        r::Application::quit = true;
    }
}

static void _destroy_window_system() noexcept
{
    CloseWindow();
}

/**
 * public
*/

r::WindowPlugin::WindowPlugin(const WindowPluginConfig &config) noexcept : _config(config)
{
    if (_config.disable_raylib_log) {
        SetTraceLogLevel(LOG_NONE);
    }
}

// clang-format off

void r::WindowPlugin::build(r::Application &app) noexcept
{
    Cursor initial_cursor_state;
    initial_cursor_state.state = _config.cursor;

    app
        .insert_resource(_config)
        .insert_resource(initial_cursor_state)
        .add_systems<_init_window_system>(Schedule::PRE_STARTUP)
        .add_systems<_update_window_system, _update_cursor_system>(Schedule::UPDATE)
        .add_systems<_destroy_window_system>(Schedule::SHUTDOWN);

    Logger::info("WindowPlugin built");
}
