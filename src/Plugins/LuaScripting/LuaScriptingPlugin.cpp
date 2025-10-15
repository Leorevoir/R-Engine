#include <R-Engine/Plugins/LuaScriptingPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <lua.hpp>

/**
 * public
 */

r::LuaScriptingPlugin::LuaScriptingPlugin(const LuaScriptingPluginConfig &config) noexcept : _config(config)
{
    /* __ctor__ */
}

void r::LuaScriptingPlugin::build(Application &app) noexcept
{
    app.insert_resource(LuaScripts{});

    Logger::info("LuaScriptingPlugin built.");
}
