#include <R-Engine/Plugins/RenderPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>

/**
 * static helpers
 */

static constexpr inline const ::Camera to_raylib(const r::Camera3d &c) noexcept
{
    return {
        .position = {c.position.x, c.position.y, c.position.z},
        .target = {c.target.x, c.target.y, c.target.z},
        .up = {c.up.x, c.up.y, c.up.z},
        .fovy = c.fovy,
        .projection = static_cast<i32>(c.projection),
    };
}

static void render_plugin_2D_before_render_system(void) noexcept
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
}

static void render_plugin_3D_before_render_system(r::ecs::Res<r::Camera3d> camera) noexcept
{
    BeginMode3D(to_raylib(*camera.ptr));
}

static void render_plugin_3D_after_render_system(void) noexcept
{
    EndMode3D();
    EndDrawing();
}

/**
* public
*/

r::RenderPlugin::RenderPlugin(const r::Camera3d &camera3d) : camera(camera3d)
{
    /* __ctor__ */
}

void r::RenderPlugin::build(r::Application &app)
{
    app.insert_resource(camera)
        .add_systems(Schedule::BEFORE_RENDER_2D, render_plugin_2D_before_render_system)
        .add_systems(Schedule::BEFORE_RENDER_3D, render_plugin_3D_before_render_system)
        .add_systems(Schedule::AFTER_RENDER_3D, render_plugin_3D_after_render_system);
}
