#include <R-Engine/Plugins/Plugin.hpp>

#include <R-Engine/Application.hpp>

void r::PluginGroup::build(r::Application &app)
{
    for (const auto &plugin : _plugins) {
        plugin->build(app);
    }
}
