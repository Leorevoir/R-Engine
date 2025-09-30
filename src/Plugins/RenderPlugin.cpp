#include <R-Engine/Plugins/RenderPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>

/**
 * static helpers
 */

static constexpr inline const ::Camera to_raylib(const r::Camera3d &c)
{
    return {
        .position = {c.position.x, c.position.y, c.position.z},
        .target = {c.target.x, c.target.y, c.target.z},
        .up = {c.up.x, c.up.y, c.up.z},
        .fovy = c.fovy,
        .projection = static_cast<i32>(c.projection),
    };
}

static void render_plugin_before_render_system(r::ecs::Res<r::Camera3d> camera)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(to_raylib(*camera.ptr));
}

static void render_plugin_after_render_system(void)
{
    EndMode3D();
    EndDrawing();
}

/**
* public
*/

r::RenderPlugin::RenderPlugin(const r::Camera3d &camera) : _camera(camera)
{
    /* __ctor__ */
}

void r::RenderPlugin::build(r::Application &app)
{
    app.insert_resource(_camera)
        .add_systems(Schedule::BEFORE_RENDER, render_plugin_before_render_system)
        .add_systems(Schedule::AFTER_RENDER, render_plugin_after_render_system);
}
