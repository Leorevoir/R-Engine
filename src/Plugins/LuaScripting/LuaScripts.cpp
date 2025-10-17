#include <R-Engine/Plugins/LuaScriptingPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Scripts/Lua.hpp>
#include <R-Engine/Scripts/LuaApiRegistry.hpp>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

/**
 * static helpers
 */

using StatePtr = std::unique_ptr<r::lua::State, void (*)(r::lua::State *)>;

static inline StatePtr lua_script_create_lua_state(void) noexcept
{
    return {luaL_newstate(), lua_close};
}

static inline void lua_script_open_standard_libraries(r::lua::State *L) noexcept
{
    luaL_openlibs(L);
}

static inline bool lua_script_dofile(r::lua::State *L, const std::string &file) noexcept
{
    return luaL_dofile(L, file.c_str()) == LUA_OK;
}

static inline const std::string lua_script_to_string(r::lua::State *L, i32 index) noexcept
{
    return lua_tostring(L, index);
}

/**
 * public
 */

r::LuaScripts::LuaScripts(Application *app) : _app_ptr(app)
{
    /* __ctor__ */
}

bool r::LuaScripts::load_script(const std::string &file)
{
    if (path::exists(file) == false) {
        Logger::error("Lua script file not found: " + file);
        return false;
    }

    LuaScriptInstance instance = {lua_script_create_lua_state()};
    r::lua::State *L = instance.get_state();

    if (L == nullptr) {
        Logger::error("Failed to create new Lua state for script: " + file);
        return false;
    }

    lua_script_open_standard_libraries(L);
    _register_engine_api(L);

    if (!lua_script_dofile(L, file)) {
        Logger::error("Failed to load or execute script: " + file);
        Logger::error(lua_tostring(L, -1));
        return false;
    }

    instance._last_write_time = std::filesystem::last_write_time(file);
    _scripts.insert_or_assign(file, std::move(instance));

    Logger::info("Successfully loaded Lua script: " + file);
    return true;
}

r::lua::State *r::LuaScripts::get_state(const std::string &file)
{
    if (auto it = _scripts.find(file); it != _scripts.end()) {
        return it->second.get_state();
    }

    return nullptr;
}

/**
 * private
 */

void r::LuaScripts::_register_engine_api(lua::State *L)
{
    const auto *registry = _app_ptr->get_resource_ptr<LuaApiRegistry>();

    if (!registry) {
        Logger::warn("LuaApiRegistry resource not found.");
        return;
    }

    for (const auto &api_script : registry->get_apis()) {
        api_script->register_module(L);
    }
}
