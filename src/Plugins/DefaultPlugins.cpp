#include <R-Engine/Plugins/AudioPlugin.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/LuaScriptingPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>
#include <R-Engine/Plugins/PostProcessingPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/TransformPlugin.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

/**
* public
*/

r::DefaultPlugins::DefaultPlugins()
{
    add<WindowPlugin>();
    add<RenderPlugin>();
    add<InputPlugin>();
    add<MeshPlugin>();
    add<TransformPlugin>();
    add<AudioPlugin>();
    add<UiPlugin>();
    add<LuaScriptingPlugin>();
    add<PostProcessingPlugin>();
}

void r::DefaultPlugins::build(r::Application &app)
{
    /* Use the base class implementation which iterates and builds all stored plugins */
    PluginGroup::build(app);
}
