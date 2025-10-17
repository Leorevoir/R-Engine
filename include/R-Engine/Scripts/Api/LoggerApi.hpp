#pragma once

#include <R-Engine/Scripts/LuaApiScript.hpp>

namespace r {

namespace lua {

/**
 * @brief API descriptor for the static Logger class.
 */
class LoggerApi final : public LuaApiScript
{
    public:
        /**
        * @brief Registers the Logger module in the given Lua state.
        */
        void register_module(State *L) noexcept override;
};

}// namespace lua

}// namespace r
