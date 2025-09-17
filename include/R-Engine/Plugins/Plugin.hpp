#pragma once

namespace r {

class Application;

/**
 * @brief base interface for all plugins.
 * @details a plugin is a class that can configure an `Application` by adding
 * resources, systems, or even other plugins to it.
 *
 * each plugin must inherit from this class and implement the `build` method
*/
class Plugin
{
    public:
        virtual ~Plugin() = default;

        /**
        * @brief configure the given `Application`
        * @details it is here that the plugin should add resources, systems...
        * @param app the application reference to configure
        */
        virtual void build(Application &app) = 0;
};

/**
* @brief base interface for plugins groups.
* @details a plugin group is a container that makes it easy to add multiple
* plugins at once.
* `DefaultPlugins` is an example of a plugin group.
*/
class PluginGroup
{
    public:
        virtual ~PluginGroup() = default;

        /**
        * @brief configure the given `Application` by adding multiple plugins to it.
        * @details it is here that the plugin group should add multiple plugins to
        * the application.
        * @param app the application reference to configure
        */
        virtual void build(Application &app) = 0;
};

}// namespace r
