#pragma once

#include <R-Engine/R-EngineExport.hpp>

#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Flagable.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Scene.hpp>
#include <R-Engine/ECS/System.hpp>

#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace r {

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
};
R_ENUM_FLAGABLE(Schedule)

// clang-format on

class R_ENGINE_API Application final
{
    private:
        /**
         * @brief Helper struct to generate a unique type for each unique system function.
         * @details `SystemTag<func1>` and `SystemTag<func2>` are distinct types,
         * so `typeid(SystemTag<func1>)` will be different from `typeid(SystemTag<func2>)`.
         */
        template<auto Func>
        struct SystemTag {
        };

        /**
         * @brief Helper struct to generate a unique type for each unique system set.
         * @details `SystemSetTag<SetType>` allows creating distinct type_index for each set.
         */
        template<typename T>
        struct SystemSetTag {
        };

        using SystemTypeId = std::type_index;
        using SystemSetId = std::type_index;
        using SystemFn = void (*)(ecs::Scene &, ecs::CommandBuffer &);

        struct SystemNode {
                SystemNode();
                SystemNode(const std::string &p_name, SystemTypeId p_id, SystemFn p_func, std::vector<SystemTypeId> p_dependencies);

                std::string name;
                SystemTypeId id;
                SystemFn func = nullptr;
                std::vector<SystemTypeId> dependencies;
                std::vector<SystemSetId> member_of_sets;
                std::vector<SystemSetId> after_sets;
                std::vector<SystemSetId> before_sets;
        };

        /**
         * @brief Represents a named group of systems.
         * @details Systems can belong to multiple sets. Sets can have ordering constraints
         * relative to other sets using before_sets and after_sets.
         */
        struct SystemSet {
                SystemSet(const std::string &pname, SystemSetId pid) noexcept;

                std::string name;
                SystemSetId id;
                std::vector<SystemSetId> before_sets;
                std::vector<SystemSetId> after_sets;
        };

        struct ScheduleGraph {
                std::unordered_map<SystemTypeId, SystemNode> nodes;
                std::unordered_map<SystemSetId, SystemSet> sets;
                std::vector<const SystemNode *> execution_order;
                bool dirty = true;
        };

        using ScheduleMap = std::unordered_map<Schedule, ScheduleGraph>;

    public:
        /* Forward declaration */
        template<typename... SetTypes>
        class SetConfigurator;

        /**
        * @brief A builder object for configuring system execution order.
        * @details Returned by Application::add_systems(). It allows for chaining
        * calls like .after() and .before() to specify dependencies.
        * It also forwards other Application builder methods to continue the chain.
        */
        class SystemConfigurator
        {
            public:
                SystemConfigurator(Application *app, Schedule schedule, std::vector<SystemTypeId> system_ids) noexcept;

                /**
                * @brief Specifies that the recently added systems must run after a given system.
                * @tparam SystemFunc The system function to run after.
                * @return A reference to this SystemConfigurator for chaining.
                */
                template<auto SystemFunc>
                SystemConfigurator &after() noexcept;

                /**
                 * @brief Specifies that the recently added systems must run after a given set.
                 * @tparam SetType The system set to run after.
                 * @return A reference to this SystemConfigurator for chaining.
                 */
                template<typename SetType>
                SystemConfigurator &after() noexcept;

                /**
                * @brief Specifies that the recently added systems must run before a given system.
                * @tparam SystemFunc The system function to run before.
                * @return A reference to this SystemConfigurator for chaining.
                */
                template<auto SystemFunc>
                SystemConfigurator &before() noexcept;

                /**
                 * @brief Specifies that the recently added systems must run before a given set.
                 * @tparam SetType The system set to run before.
                 * @return A reference to this SystemConfigurator for chaining.
                 */
                template<typename SetType>
                SystemConfigurator &before() noexcept;

                /**
                * @brief Adds the recently added systems to a named set.
                * @details Systems in the same set can be ordered as a group using configure_sets().
                * A system can belong to multiple sets.
                * @tparam SetType The type representing the set (e.g., struct PhysicsSet {};)
                * @return A reference to this SystemConfigurator for chaining.
                */
                template<typename SetType>
                SystemConfigurator &in_set() noexcept;

                /**
                * @brief Forwards to Application::add_systems to continue adding more systems.
                */
                template<auto... SystemFuncs>
                SystemConfigurator add_systems(Schedule when) noexcept;

                /**
                 * @brief Forwards to Application::configure_sets to start configuring sets.
                 */
                template<typename... SetTypes>
                SetConfigurator<SetTypes...> configure_sets(Schedule when) noexcept;

                /**
                * @brief Forwards to Application::insert_resource and returns this configurator.
                */
                template<typename ResT>
                SystemConfigurator &insert_resource(ResT res) noexcept;

                /**
                * @brief Forwards to Application::add_plugins and returns this configurator.
                */
                template<typename... Plugins>
                SystemConfigurator &add_plugins(Plugins &&...plugins) noexcept;

                /**
                * @brief Forwards to Application::run() to start the application.
                */
                void run();

            private:
                Application *_app;
                Schedule _schedule;
                std::vector<SystemTypeId> _system_ids;
        };

        /**
         * @brief A builder object for configuring set ordering constraints.
         * @details Returned by Application::configure_sets(). Allows specifying
         * that entire groups of systems must run before or after other groups.
         */
        template<typename... SetTypes>
        class SetConfigurator
        {
            public:
                SetConfigurator(Application *app, Schedule schedule, std::vector<SystemSetId> setids) noexcept;

                /**
                 * @brief Specifies that all systems in the configured sets must run before
                 * all systems in the target set.
                 * @tparam OtherSet The set to run before.
                 * @return A reference to this SetConfigurator for chaining.
                 */
                template<typename OtherSet>
                SetConfigurator &before() noexcept;

                /**
                 * @brief Specifies that all systems in the configured sets must run after
                 * all systems in the target set.
                 * @tparam OtherSet The set to run after.
                 * @return A reference to this SetConfigurator for chaining.
                 */
                template<typename OtherSet>
                SetConfigurator &after() noexcept;

                /**
                * @brief Forwards to Application::add_systems to continue adding more systems.
                */
                template<auto... SystemFuncs>
                SystemConfigurator add_systems(Schedule when) noexcept;

                /**
                 * @brief Forwards to Application::configure_sets to continue configuring sets.
                 */
                template<typename... OtherSetTypes>
                SetConfigurator<OtherSetTypes...> configure_sets(Schedule when) noexcept;

                /**
                * @brief Forwards to Application::insert_resource and returns this configurator.
                */
                template<typename ResT>
                SetConfigurator &insert_resource(ResT res) noexcept;

                /**
                * @brief Forwards to Application::add_plugins and returns this configurator.
                */
                template<typename... Plugins>
                SetConfigurator &add_plugins(Plugins &&...plugins) noexcept;

                /**
                * @brief Forwards to Application::run() to start the application.
                */
                void run();

            private:
                Application *_app;
                Schedule _schedule;
                std::vector<SystemSetId> _set_ids;
        };

        Application();
        ~Application() = default;

        /**
        * @brief Adds one or more systems to the application schedule.
        * @details Returns a SystemConfigurator to allow for chaining calls like .after()
        * or .before() to define execution order dependencies.
        * @param when The schedule to run the systems in.
        * @param SystemFuncs The system functions to add.
        * @return A SystemConfigurator instance for dependency configuration.
        */
        template<auto... SystemFuncs>
        SystemConfigurator add_systems(Schedule when) noexcept;

        /**
         * @brief Configures ordering constraints for system sets.
         * @details Use this to define the execution order of entire groups of systems.
         * Sets must be represented as distinct types (e.g., struct PhysicsSet {};).
         * @tparam SetTypes The set types to configure.
         * @param when The schedule in which to configure the sets.
         * @return A SetConfigurator instance for ordering configuration.
         */
        template<typename... SetTypes>
        SetConfigurator<SetTypes...> configure_sets(Schedule when) noexcept;

        /**
        * @brief Inserts a resource into the Application scene.
        * @details Resources are unique, global data structures accessible by systems.
        * @param res The resource to insert.
        * @return A reference to the Application for chaining.
        */
        template<typename ResT>
        Application &insert_resource(ResT res) noexcept;

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

        static inline std::atomic_bool quit{false};

    private:
        void _startup();
        void _main_loop();
        void _shutdown();
        void _render_routine();
        void _run_schedule(const Schedule sched);
        void _sort_schedule(ScheduleGraph &graph);
        void _execute_systems(const ScheduleGraph &graph);
        void _apply_commands();

        template<auto SystemFunc>
        SystemTypeId _add_one_system(Schedule when) noexcept;

        template<typename SetType>
        SystemSetId _add_one_set(Schedule when) noexcept;

        template<typename PluginT>
        void _add_one_plugin(PluginT &&plugin) noexcept;

        core::Clock _clock = {};
        ScheduleMap _systems = {};
        ecs::Scene _scene = {};
        ecs::CommandBuffer _command_buffer = {};
};

}// namespace r

#include "Application.inl"
