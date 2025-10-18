#pragma once

#include <R-Engine/Scripts/Lua.hpp>

namespace r {

/**
 * @brief interface for any C++ object wishing to expose an API to Lua.
 */
class R_ENGINE_API LuaApiScript
{
    public:
        virtual ~LuaApiScript() = default;

        /**
         * @brief save the module's functions into the given Lua state.
         * @param L the Lua state to register the API into.
         */
        virtual void register_module(lua::State *L) noexcept = 0;
};

}// namespace r
