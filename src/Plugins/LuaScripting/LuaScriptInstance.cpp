#include <R-Engine/Plugins/LuaScriptingPlugin.hpp>

/**
* public
*/

r::LuaScriptInstance::LuaScriptInstance() noexcept : _state(nullptr, nullptr)
{
    /* __ctor__ */
}

r::LuaScriptInstance::LuaScriptInstance(std::unique_ptr<lua::State, void (*)(lua::State *)> state) noexcept
    : _state(std::move(state)), _last_write_time{}
{
    /* __ctor__ */
}

r::LuaScriptInstance::LuaScriptInstance(LuaScriptInstance &&other) noexcept
    : _state(std::move(other._state)), _last_write_time(other._last_write_time)
{
    /* __ctor__ */
}

r::LuaScriptInstance &r::LuaScriptInstance::operator=(LuaScriptInstance &&other) noexcept
{
    if (this != &other) {
        _state = std::move(other._state);
        _last_write_time = other._last_write_time;
    }
    return *this;
}

r::lua::State *r::LuaScriptInstance::get_state() const
{
    return _state.get();
}
