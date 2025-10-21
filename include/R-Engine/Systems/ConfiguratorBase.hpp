#pragma once

namespace r {

class Application;
enum class Schedule;

namespace sys {

template<typename... SetTypes>
class SetConfigurator;
class SystemConfigurator;

/**
* @brief Base class for configurators to remove duplicate forwarding methods.
* @details Uses the Curiously Recurring Template Pattern (CRTP).
*/
template<typename Derived>
class ConfiguratorBase
{
    protected:
        Application *_app;

        ConfiguratorBase(Application *app) noexcept : _app(app)
        {
            /* __ctor__ */
        }

    public:
        template<auto... SystemFuncs, typename ScheduleLabel>
        SystemConfigurator add_systems(ScheduleLabel label) noexcept;

        /**
         * @brief Forwards to Application::configure_sets to start configuring sets.
         */
        template<typename... SetTypes>
        SetConfigurator<SetTypes...> configure_sets(r::Schedule when) noexcept;

        /**
        * @brief Forwards to Application::insert_resource and returns this configurator.
        */
        template<typename ResT>
        Derived &insert_resource(ResT &&res) noexcept;

        /**
        * @brief Forwards to Application::add_plugins and returns this configurator.
        */
        template<typename... Plugins>
        Derived &add_plugins(Plugins &&...plugins) noexcept;

        /**
        * @brief Forwards to Application::run() to start the application.
        */
        void run();
};

}// namespace sys

}// namespace r
