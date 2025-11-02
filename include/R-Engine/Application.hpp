#pragma once

#include "R-Engine/Core/ThreadPool.hpp"
#include "R-Engine/Systems/Scheduler.hpp"
#include <R-Engine/R-EngineExport.hpp>

#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Flagable.hpp>

#include <R-Engine/ECS/RunConditions.hpp>
#include <R-Engine/ECS/System.hpp>

#include <R-Engine/Systems/ConfiguratorBase.hpp>
#include <R-Engine/Systems/ScheduleGraph.hpp>
#include <R-Engine/Systems/SetConfigurator.hpp>
#include <R-Engine/Systems/States.hpp>
#include <R-Engine/Systems/SystemConfigurator.hpp>

namespace r {

namespace sys {

template<typename...>
class SetConfigurator;
class SystemConfigurator;

}// namespace sys

// clang-format off

enum class Schedule {
    PRE_STARTUP      = 1 << 0,
    STARTUP          = 1 << 1,
    UPDATE           = 1 << 2,
    FIXED_UPDATE     = 1 << 3,
    BEFORE_RENDER_2D = 1 << 4,
    RENDER_2D        = 1 << 5,
    AFTER_RENDER_2D  = 1 << 6,
    BEFORE_RENDER_3D = 1 << 7,
    RENDER_3D        = 1 << 8,
    AFTER_RENDER_3D  = 1 << 9,
    SHUTDOWN         = 1 << 10,
    EVENT_CLEANUP    = 1 << 11
};
R_ENUM_FLAGABLE(Schedule)

// clang-format on

class R_ENGINE_API Application final
{
    private:
        std::unordered_map<std::type_index, sys::States> _states;
        std::vector<std::function<void()>> _state_transition_runners;

        using ScheduleMap = std::unordered_map<Schedule, sys::ScheduleGraph>;
        friend class sys::SystemConfigurator;
        template<typename...>
        friend class sys::SetConfigurator;

    public:
        Application();
        ~Application() = default;

        /**
        * @brief Inits the first state
        */
        template<typename T>
        Application &init_state(T initial_state) noexcept;

        /**
        * @brief Adds one or more systems to the application schedule.
        * @details Returns a sys::SystemConfigurator to allow for chaining calls like .after()
        * or .before() to define execution order dependencies.
        * @param label The schedule or state event to run the systems in.
        * @param SystemFuncs The system functions to add.
        * @return A sys::SystemConfigurator instance for dependency configuration.
        */
        template<auto... SystemFuncs, typename ScheduleLabel>
        sys::SystemConfigurator add_systems(ScheduleLabel label);

        /**
         * @brief Configures ordering constraints for system sets.
         * @details Use this to define the execution order of entire groups of systems.
         * Sets must be represented as distinct types (e.g., struct PhysicsSet {};).
         * @tparam SetTypes The set types to configure.
         * @param when The schedule in which to configure the sets.
         * @return A sys::SetConfigurator instance for ordering configuration.
         */
        template<typename... SetTypes>
        sys::SetConfigurator<SetTypes...> configure_sets(Schedule when) noexcept;

        /**
        * @brief Inserts a resource into the Application scene.
        * @details Resources are unique, global data structures accessible by systems.
        * @param res The resource to insert.
        * @return A reference to the Application for chaining.
        */
        template<typename ResT>
        Application &insert_resource(ResT &&res) noexcept;

        /**
         * @brief Adds one or more plugins to the application.
         * @details This is the primary way to add functionality to an `Application`.
         * Plugins can be added individually or in groups.
         * @param plugins Plugin instances to add.
         * @return A reference to the Application for chaining.
         */
        template<typename... Plugins>
        Application &add_plugins(Plugins &&...plugins) noexcept;

        /**
         * @brief add EventT(s) to the application
         * @details also adds a clear_events_sytem foreach EventT to the EVENT_CLEANUP schedule
         */
        template<typename... EventTs>
        Application &add_events(void) noexcept;

        /**
        * @brief run the application
        * @details this will start the main loop of the application
        *
        * PRE_STARTUP systems();
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

        /**
         * @brief Gets a pointer to a resource from the application's scene.
         * @details Useful for inspecting state, especially in tests.
         * @tparam T The type of the resource to get.
         * @return A pointer to the resource, or nullptr if it doesn't exist.
         */
        template<typename T>
        T *get_resource_ptr() noexcept;

        /**
        * @brief Call the startup function (useful for the server)
        * @details Function for the server which is calling the startup function
        */
        void init(); 

        /**
        * @brief Call every useful functions for the game server loop
        * @details Is used by the server to call every functions useful functions for the game server loop
        */
        void tick();

        static inline std::atomic_bool quit{false};
        static inline std::atomic_bool quit_from_signal{false};

    private:
        void _startup();
        void _main_loop();
        void _shutdown();
        void _render_routine();
        void _run_schedule(const Schedule sched);
        void _apply_commands();
        void _apply_state_transitions();
        void _run_transition_schedule(sys::ScheduleGraph &graph);
        void _prepare_thread_local_buffers(size_t count);

        template<typename SetType>
        sys::SystemSetId _ensure_set_exists(sys::ScheduleGraph &graph) noexcept;

        template<typename PluginT>
        void _add_one_plugin(PluginT &&plugin) noexcept;

        template<auto SystemFunc>
        sys::SystemTypeId _add_one_system_to_graph(sys::ScheduleGraph &graph, bool main_thread_only) noexcept;

        core::Clock _clock = {};
        ScheduleMap _systems = {};
        ecs::Scene _scene = {};
        ecs::CommandBuffer _command_buffer = {};

        std::unique_ptr<core::ThreadPool> _thread_pool;
        std::unique_ptr<core::Scheduler> _scheduler;
        std::vector<std::unique_ptr<ecs::CommandBuffer>> _thread_local_command_buffers;
};

}// namespace r

#include "Application.inl"
#include "Systems/Inline/ConfiguratorBase.inl"
#include "Systems/Inline/SetConfigurator.inl"
#include "Systems/Inline/SystemConfigurator.inl"
