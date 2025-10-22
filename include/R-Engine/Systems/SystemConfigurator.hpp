#pragma once

#include <R-Engine/Systems/ConfiguratorBase.hpp>
#include <R-Engine/Systems/ScheduleGraph.hpp>

namespace r {

namespace sys {

/**
* @brief A builder object for configuring system execution order.
* @details Returned by Application::add_systems(). It allows for chaining
* calls like .after() and .before() to specify dependencies.
* It also forwards other Application builder methods to continue the chain.
*/
class SystemConfigurator final : public ConfiguratorBase<SystemConfigurator>
{
    public:
        SystemConfigurator(Application *app, ScheduleGraph *graph, std::vector<SystemTypeId> system_ids) noexcept;

        using ConfiguratorBase<SystemConfigurator>::add_systems;

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
         * @brief Specifies a condition that must be met for the systems to run.
         * @details The provided function is a "predicate" that will be called
         * before the systems are executed. If it returns false, the systems are skipped.
         * The predicate can take any valid system parameters (e.g., Res<State<T>>).
         * @tparam PredicateFunc The predicate function.
         * @return A reference to this SystemConfigurator for chaining.
         */
        template<auto PredicateFunc>
        SystemConfigurator &run_if() noexcept;

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
         * @brief Adds a condition that must also be true (logical AND).
         * @details If no condition exists, this behaves like run_if.
         * @tparam PredicateFunc The predicate to AND with the existing condition.
         * @return A reference to this SystemConfigurator for chaining.
         */
        template<auto PredicateFunc>
        SystemConfigurator &run_and() noexcept;

        /**
         * @brief Adds a condition that can also be true (logical OR).
         * @details If no condition exists, this behaves like run_if.
         * @tparam PredicateFunc The predicate to OR with the existing condition.
         * @return A reference to this SystemConfigurator for chaining.
         */
        template<auto PredicateFunc>
        SystemConfigurator &run_or() noexcept;

        /**
         * @brief Sets a condition that must NOT be met for the systems to run.
         * @details This overwrites any previous conditions.
         * @tparam PredicateFunc The predicate to negate.
         * @return A reference to this SystemConfigurator for chaining.
         */
        template<auto PredicateFunc>
        SystemConfigurator &run_unless() noexcept;

    private:
        ScheduleGraph *_graph;
        std::vector<SystemTypeId> _system_ids;

        std::function<bool(r::ecs::Scene &)> _current_condition;

        template<auto PredicateFunc>
        auto _create_condition_wrapper(bool negated = false) -> std::function<bool(ecs::Scene &)>;

        void _apply_condition();
};

}// namespace sys

}// namespace r
