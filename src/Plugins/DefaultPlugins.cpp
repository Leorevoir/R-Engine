#include <R-Engine/Plugins/DefaultPlugins.hpp>

#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

#include <R-Engine/Application.hpp>

/**
* public
*/

r::DefaultPlugins::DefaultPlugins()
{
    add<WindowPlugin>();
    add<RenderPlugin>();
    add<InputPlugin>();
    add<MeshPlugin>();
}

void r::DefaultPlugins::build(r::Application &app)
{
    /* Use the base class implementation which iterates and builds all stored plugins */
    PluginGroup::build(app);
}
