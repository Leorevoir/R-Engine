#pragma once

#include <R-Engine/Scripts/Lua.hpp>

namespace r {

namespace lua {

/**
 * @brief RAII wrapper to ensure the Lua stack is balanced automatically.
 * @details On construction, it records the current stack top.
 * On destruction (when it goes out of scope), it restores the stack to that
 * original state, preventing stack leaks or corruption from unpopped values.
 */
class StackGuard final : public NonCopyable
{
    public:
        explicit StackGuard(State *L);
        ~StackGuard();

    private:
        State *_state;
        int _top;
};

}// namespace lua

}// namespace r
