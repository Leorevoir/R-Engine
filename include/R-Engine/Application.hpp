#pragma once

#include <R-Engine/R-EngineExport.hpp>

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

class R_ENGINE_API Application
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
         * @brief Adds one or more plugins to the application.
         * @details This is the primary way to add functionality to an `Application`.
         * Plugins can be added individually or in groups.
         * This method can be chained.
         * @param plugins Plugin instances to add.
         */
        template<typename... Plugins>
        Application &add_plugins(Plugins &&...plugins) noexcept;

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

        template<typename PluginT>
        void _add_one_plugin(PluginT &&plugin) noexcept;

        using SystemFunc = std::function<void(ecs::Scene &, ecs::CommandBuffer &)>;
        using ScheduleMap = std::unordered_map<Schedule, std::vector<SystemFunc>>;

        core::Clock _clock = {};
        ScheduleMap _systems = {};
        ecs::Scene _scene = {};
        ecs::CommandBuffer _command_buffer = {};
};

}// namespace r

#include "Application.inl"
