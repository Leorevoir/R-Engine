#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Scripts/Api/LoggerApi.hpp>

extern "C" {
#include <lauxlib.h>
}

/**
 * static helpers
 */

static int lua_register_mdodule_log_debug(r::lua::State *L)
{
    r::Logger::debug(r::lua::get<std::string_view>(L, 1));
    return 0;
}

static int lua_register_mdodule_log_info(r::lua::State *L)
{
    r::Logger::info(r::lua::get<std::string_view>(L, 1));
    return 0;
}

static int lua_register_mdodule_log_warn(r::lua::State *L)
{
    r::Logger::warn(r::lua::get<std::string_view>(L, 1));
    return 0;
}

static int lua_register_mdodule_log_error(r::lua::State *L)
{
    r::Logger::error(r::lua::get<std::string_view>(L, 1));
    return 0;
}

// clang-format off

static constexpr r::lua::CFunctionRegistry log_functions[] =
{
    {"debug", lua_register_mdodule_log_debug},
    {"info", lua_register_mdodule_log_info},
    {"warn", lua_register_mdodule_log_warn},
    {"error", lua_register_mdodule_log_error},
    {NULL, NULL}
};

static inline void lua_register_module_log(r::lua::State *L)
{
    r::lua::register_module(L, "Log", log_functions);
}

/**
* public
*/

void r::lua::LoggerApi::register_module(State *L) noexcept
{
    lua_register_module_log(L);
}
