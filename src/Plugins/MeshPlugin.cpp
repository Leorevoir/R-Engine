#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

/**
* public
*/

r::MeshPlugin::MeshPlugin(const MeshPluginConfig &config) noexcept : _config(config)
{
    /* __ctor__ */
}

void r::MeshPlugin::build(r::Application &app) noexcept
{
    (void) app;
}
