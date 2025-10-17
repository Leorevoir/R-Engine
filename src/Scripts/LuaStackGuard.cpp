#include <R-Engine/Scripts/LuaStackGuard.hpp>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

/**
 * public
 */

r::lua::StackGuard::StackGuard(State *L) : _state(L), _top(lua_gettop(L))
{
    /* __ctor__ */
}

r::lua::StackGuard::~StackGuard()
{
    if (_state) {
        lua_settop(_state, _top);
    }
}
