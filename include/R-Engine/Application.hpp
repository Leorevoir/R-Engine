#pragma once

#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Flagable.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Scene.hpp>
#include <R-Engine/ECS/System.hpp>

#include <functional>
#include <vector>

namespace r {

enum class Schedule {
    STARTUP = 1 << 0,
    UPDATE = 1 << 1,
    FIXED_UPDATE = 1 << 2,
    RENDER = 1 << 3,
    SHUTDOWN = 1 << 4,
};
R_ENUM_FLAGABLE(Schedule)

class Application
{
    public:
        Application();
        ~Application() = default;

        /**
        * @brief add one or more systems to the Application
        * @details the systems will be run in the specified schedule
        * @param when the schedule to run the systems in
        * @param funcs the system functions to run
        */
        template<typename... Funcs>
        Application &add_systems(Schedule when, Funcs &&...funcs) noexcept;

        /**
        * @brief insert a resource into the Application scene
        * @param res the resource to insert
        */
        template<typename ResT>
        Application &insert_resource(ResT res) noexcept;

        /**
         * @brief add a plugin or plugin group instance to the Application.
         * @details This is used for pre-configured plugins.
         * @param plugin The plugin or plugin group instance.
         */
        template<typename PluginT>
        Application &add_plugins(PluginT &&plugin) noexcept;

        /**
        * @brief add a plugin or a plugin group to the Application by type, with optional constructor arguments.
        * @param args the arguments to pass to the plugin or plugin group constructor (can be empty)
        */
        template<typename PluginT, typename... Args>
        Application &add_plugins(Args &&...args) noexcept;

        /**
        * @brief add multiple plugins or plugin groups to the Application by type (without constructor arguments).
        */
        template<typename PluginT1, typename PluginT2, typename... Rest>
        Application &add_plugins() noexcept;

        /**
        * @brief run the application
        * @details this will start the main loop of the application
        *
        * STARTUP systems();
        *
        * <main loop> {
        *     UPDATE systems();
        *
        *     for (each fixed timestep) {
        *         FIXED_UPDATE systems();
        *     }
        * }
        *
        * SHUTDOWN systems();
        */
        void run();

        static inline bool quit = false;

    private:
        void _run_schedule(const Schedule sched);
        void _apply_commands();

        template<typename Func>
        void _add_one_system(Schedule when, Func &&func) noexcept;

        using SystemFunc = std::function<void(ecs::Scene &, ecs::CommandBuffer &)>;
        using ScheduleMap = std::unordered_map<Schedule, std::vector<SystemFunc>>;

        core::Clock _clock = {};
        ScheduleMap _systems = {};
        ecs::Scene _scene = {};
        ecs::CommandBuffer _command_buffer = {};
};

}// namespace r

#include "Application.inl"
