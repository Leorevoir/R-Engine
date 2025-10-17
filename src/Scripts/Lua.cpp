#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Scripts/Lua.hpp>
#include <R-Engine/Scripts/LuaStackGuard.hpp>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

/**
* get
*/

template<>
bool r::lua::get<bool>(State *L, int index)
{
    return lua_toboolean(L, index) != 0;
}

template<>
std::string r::lua::get<std::string>(State *L, int index)
{
    size_t len;
    const char *str = luaL_checklstring(L, index, &len);

    return std::string(str, len);
}

template<>
std::string_view r::lua::get<std::string_view>(State *L, int index)
{
    size_t len;
    const char *str = luaL_checklstring(L, index, &len);

    return std::string_view(str, len);
}

/**
* push
*/

void r::lua::push(State *L, bool value)
{
    lua_pushboolean(L, value);
}

void r::lua::push(State *L, const std::string &value)
{
    lua_pushlstring(L, value.c_str(), value.length());
}

void r::lua::push(State *L, std::string_view value)
{
    lua_pushlstring(L, value.data(), value.length());
}

/**
* static helpers
*/

static inline void lua_register_module(r::lua::State *L, const std::string &table_name,
    const r::lua::CFunctionRegistry functions[]) noexcept
{
    lua_newtable(L);
    luaL_setfuncs(L, functions, 0);
    lua_setglobal(L, table_name.c_str());
}

/**
* register
*/

void r::lua::register_module(State *L, const std::string &table_name, const CFunctionRegistry functions[])
{
    StackGuard guard(L);

    lua_register_module(L, table_name, functions);
}
