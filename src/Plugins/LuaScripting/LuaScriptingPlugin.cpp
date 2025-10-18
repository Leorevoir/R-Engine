#include <R-Engine/Plugins/LuaScriptingPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Filepath.hpp>
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
 * static helper
 */

static inline void lua_scripting_plugin_register_api(r::LuaApiRegistry &registry) noexcept
{
    registry.add_api(std::make_shared<r::lua::LoggerApi>());
}

static inline bool lua_scripting_plugin_is_script_instance_ready(const r::LuaScriptInstance *instance, const std::string &file) noexcept
{
    if (!r::path::exists(file)) {
        return false;
    }

    const auto last_write_time = r::path::last_write_time(file);

    return !instance || instance->get_last_write_time() != last_write_time;
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
        const std::string &file_path = script_component.ptr->file_path;

        if (file_path.empty() || !r::path::exists(file_path)) {
            continue;
        }

        const auto *script_instance = scripts_manager.ptr->get_script_instance(file_path);

        if (!lua_scripting_plugin_is_script_instance_ready(script_instance, file_path)) {
            continue;
        }

        r::Logger::debug("Hot-reloading Lua script: " + file_path);
        if (scripts_manager.ptr->load_script(file_path)) {
            script_component.ptr->ready = true;
            continue;
        }

        r::Logger::error("Failed to hot-reload script: " + file_path);
        script_component.ptr->ready = false;
    }
}

}// namespace

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

    Logger::debug("LuaScriptingPlugin built");
}
