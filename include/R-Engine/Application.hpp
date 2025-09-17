#pragma once

#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Flagable.hpp>
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
        * @brief add a system to the Application
        * @details the system will be run in the specified schedule
        * @param when the schedule to run the system in
        * @param func the system function to run
        */
        template<typename Func>
        Application &add_system(Schedule when, Func &&func) noexcept;

        /**
        * @brief insert a resource into the Application scene
        * @param res the resource to insert
        */
        template<typename ResT>
        Application &insert_resource(ResT res) noexcept;

        /**
        * @brief add a plugin or a plugin group to the Application
        * @param args the arguments to pass to the plugin or plugin group constructor (can be empty)
        */
        template<typename PluginT, typename... Args>
        Application &add_plugins(Args &&...args) noexcept;

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

        using SystemFunc = std::function<void(ecs::Scene &)>;
        using ScheduleMap = std::unordered_map<Schedule, std::vector<SystemFunc>>;

        core::Clock _clock = {};
        ScheduleMap _systems = {};
        ecs::Scene _scene = {};
};

}// namespace r

#include "Application.inl"
