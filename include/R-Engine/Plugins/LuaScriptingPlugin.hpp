#pragma once

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>

#include <R-Engine/Scripts/Lua.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace r {

/**
 * @brief Represents a Lua script file.
 */
struct R_ENGINE_API Script {
        std::string file_path;
        bool ready = false;
};

/**
 * @brief Represents an instance of a loaded Lua script.
 * @details This class manages the Lua state for a single script and tracks its last modification time.
 */
class R_ENGINE_API LuaScriptInstance final : public MoveOnly
{
    public:
        using DeleteFunc = void (*)(lua::State *);
        using StatePtr = std::unique_ptr<lua::State, DeleteFunc>;

        LuaScriptInstance() noexcept;
        LuaScriptInstance(StatePtr state) noexcept;
        LuaScriptInstance(LuaScriptInstance &&other) noexcept;
        LuaScriptInstance &operator=(LuaScriptInstance &&other) noexcept;

        lua::State *get_state() const;
        std::filesystem::file_time_type get_last_write_time() const;

    private:
        friend class LuaScripts;

        StatePtr _state;
        std::filesystem::file_time_type _last_write_time;
};

/**
 * @brief A resource that manages all loaded Lua scripts.
 * @details This class is the central hub for loading scripts and registering the C++ API.
 */
class R_ENGINE_API LuaScripts final : public MoveOnly
{
    public:
        explicit LuaScripts(Application *app);

        bool load_script(const std::string &path);
        lua::State *get_state(const std::string &path);
        LuaScriptInstance *get_script_instance(const std::string &path);

    private:
        void _register_engine_api(lua::State *L);

        std::unordered_map<std::string, LuaScriptInstance> _scripts;
        Application *_app_ptr;
};

/**
* @brief Configuration structure for LuaScriptingPlugin.
*/
// INFO: keep commented out until configuration options are added
// struct R_ENGINE_API LuaScriptingPluginConfig {
// };

/**
* @brief Plugin that adds Lua scripting support to the application.
*/
class R_ENGINE_API LuaScriptingPlugin final : public Plugin
{
    public:
        // explicit LuaScriptingPlugin(const LuaScriptingPluginConfig &config = LuaScriptingPluginConfig()) noexcept;
        LuaScriptingPlugin() noexcept = default;
        void build(Application &app) noexcept override;

    private:
        // INFO: keep commented out until configuration options are added
        // LuaScriptingPluginConfig _config;
};

}// namespace r
