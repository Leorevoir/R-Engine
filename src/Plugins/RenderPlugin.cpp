#include <R-Engine/Plugins/RenderPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>

/**
 * helpers
 */

static void _begin_frame(void)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
}

static void _end_frame(void)
{
    EndDrawing();
}

/**
* public
*/

void r::RenderPlugin::build(r::Application &app)
{
    app.add_system(Schedule::RENDER, _begin_frame);
    app.add_system(Schedule::RENDER, _end_frame);
}
