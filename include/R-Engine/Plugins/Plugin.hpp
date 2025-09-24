#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <memory>
#include <vector>

namespace r {

class Application;

/**
 * @brief base interface for all plugins.
 * @details a plugin is a class that can configure an `Application` by adding
 * resources, systems, or even other plugins to it.
 *
 * each plugin must inherit from this class and implement the `build` method
*/
class R_ENGINE_API Plugin
{
    public:
        Plugin() = default;
        virtual ~Plugin() = default;

        Plugin(Plugin &&) = default;
        Plugin &operator=(Plugin &&) = default;
        Plugin(const Plugin &) = default;
        Plugin &operator=(const Plugin &) = default;

        /**
        * @brief configure the given `Application`
        * @details it is here that the plugin should add resources, systems...
        * @param app the application reference to configure
        */
        virtual void build(Application &app) = 0;
};

/**
* @brief A collection of plugins that can be added to an `Application` together.
* @details Plugin groups can be configured using the `.set()` method to replace
* a default plugin with a custom-configured one.
*/
class R_ENGINE_API PluginGroup
{
    public:
        PluginGroup() = default;
        virtual ~PluginGroup() = default;

        /**
        * @brief Builds all plugins in the group.
        * @param app The application to configure.
        */
        virtual void build(Application &app);

        /**
         * @brief Replaces a plugin of the same type `PluginT` within the group,
         * or adds it if it's not already present.
         * @tparam PluginT The type of the plugin to set.
         * @param plugin An instance of the plugin to add/replace.
         * @return A reference to this `PluginGroup` to allow for chaining.
         */
        template<typename PluginT>
        PluginGroup &set(PluginT plugin);

    protected:
        /**
         * @brief Adds a plugin to the group. Intended for use by derived classes in their constructors.
         * @tparam PluginT The type of plugin to add.
         * @tparam Args Constructor arguments for the plugin.
         */
        template<typename PluginT, typename... Args>
        void add(Args &&...args);

    private:
        std::vector<std::shared_ptr<Plugin>> _plugins;
};

}// namespace r

#include "Inline/Plugin.inl"
