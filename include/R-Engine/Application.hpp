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
    SHUTDOWN = 1 << 3,
};
R_ENUM_FLAGABLE(Schedule)

class Application
{
    public:
        Application();
        ~Application() = default;

        template<typename Func>
        Application &add_system(Schedule when, Func &&func) noexcept;

        template<typename ResT>
        Application &insert_resource(ResT res) noexcept;

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
