#include <R-Engine/Plugins/DefaultPlugins.hpp>

#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

#include <R-Engine/Application.hpp>

/**
* public
*/

void r::DefaultPlugins::build(r::Application &app)
{
    app.add_plugins<r::WindowPlugin>();
    app.add_plugins<r::RenderPlugin>();
}
