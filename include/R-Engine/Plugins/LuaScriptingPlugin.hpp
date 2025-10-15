#pragma once

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>

#include <filesystem>
#include <unordered_map>

struct lua_State;

namespace r {

struct R_ENGINE_API Script {
        std::string file_path;
        bool ready = false;
};

class R_ENGINE_API LuaScriptInstance
{
    public:
    private:
        std::unique_ptr<lua_State, void (*)(lua_State *)> _state;
        std::filesystem::file_time_type _last_write_time;
};

class R_ENGINE_API LuaScripts
{
    private:
        std::unordered_map<std::string, LuaScriptInstance> _scripts;
};

/**
 * @brief Configuration for the LuaScriptingPlugin.
 */
struct R_ENGINE_API LuaScriptingPluginConfig {
};

/**
 * @brief Adds Lua scripting and hot-reloading capabilities to the application.
 */
class R_ENGINE_API LuaScriptingPlugin final : public Plugin
{
    public:
        explicit LuaScriptingPlugin(const LuaScriptingPluginConfig &config = LuaScriptingPluginConfig()) noexcept;

        void build(Application &app) noexcept override;

    private:
        LuaScriptingPluginConfig _config;
};

}// namespace r
