#include <R-Engine/Plugins/LuaScriptingPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Scripts/LuaApiRegistry.hpp>

#include <R-Engine/Scripts/Api/LoggerApi.hpp>

/**
* lua api
*/

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

/**
* systems
*/

namespace {

using ScriptQuery = r::ecs::Query<r::ecs::Mut<r::Script>>;
using LuaScriptsRes = r::ecs::ResMut<r::LuaScripts>;

static void lua_scripting_plugin_script_loader(ScriptQuery query, LuaScriptsRes scripts_manager) noexcept
{
    for (auto [script_component] : query) {

        if (!script_component.ptr->ready) {

            if (scripts_manager.ptr->load_script(script_component.ptr->file_path)) {
                script_component.ptr->ready = true;
            }
        }
    }
}

}// namespace

/**
 * static helper
 */

static inline void lua_scripting_plugin_register_api(r::LuaApiRegistry &registry) noexcept
{
    registry.add_api(std::make_shared<r::lua::LoggerApi>());
}

/**
* public
*/

r::LuaScriptingPlugin::LuaScriptingPlugin(const LuaScriptingPluginConfig &config) noexcept : _config(config)
{
    /* __ctor__ */
}

void r::LuaScriptingPlugin::build(Application &app) noexcept
{
    auto registry = r::LuaApiRegistry{};

    lua_scripting_plugin_register_api(registry);

    app.insert_resource(std::move(registry));
    app.insert_resource(LuaScripts(&app));

    app.add_systems<lua_scripting_plugin_script_loader>(Schedule::UPDATE);

    Logger::info("LuaScriptingPlugin built with dynamic API registry.");
}
