#pragma once

#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Flagable.hpp>
#include <R-Engine/ECS/Scene.hpp>
#include <R-Engine/ECS/System.hpp>

#include <functional>
#include <type_traits>
#include <vector>

namespace r {

enum class Schedule {
    STARTUP = 1 << 0,
    UPDATE = 1 << 1,
    FIXED_UPDATE = 1 << 2,
};
R_ENUM_FLAGABLE(Schedule)

class Application
{
    public:
        Application() = default;
        ~Application() = default;

        template<typename Func>
        Application &add_system(Schedule when, Func &&func)
        {
            using FuncDecay = std::decay_t<Func>;
            FuncDecay fn = std::forward<Func>(func);

            _systems[when].emplace_back([fn](ecs::Scene &scene) mutable { ecs::run_system(fn, scene); });

            return *this;
        }

        template<typename ResT>
        Application &insert_resource(ResT res)
        {
            _scene.insert_resource<ResT>(std::move(res));
            return *this;
        }

        void run();

        static inline bool quit = false;

    private:
        void _run_schedule(const Schedule sched)
        {
            const auto it = _systems.find(sched);

            if (it == _systems.end()) {
                return;
            }
            for (const auto &sys : it->second) {
                sys(_scene);
            }
        }

        using SystemFunc = std::function<void(ecs::Scene &)>;
        using ScheduleMap = std::unordered_map<Schedule, std::vector<SystemFunc>>;

        core::Clock _clock = {};
        ScheduleMap _systems = {};
        ecs::Scene _scene = {};
};

}// namespace r
