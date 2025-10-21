#pragma once

#include <R-Engine/Systems/ConfiguratorBase.hpp>
#include <R-Engine/Systems/ScheduleGraph.hpp>

namespace r {

namespace sys {

/**
 * @brief A builder object for configuring set ordering constraints.
 * @details Returned by Application::configure_sets(). Allows specifying
 * that entire groups of systems must run before or after other groups.
 */
template<typename... SetTypes>
class SetConfigurator final : public ConfiguratorBase<SetConfigurator<SetTypes...>>
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

    private:
        Schedule _schedule;
        std::vector<SystemSetId> _set_ids;
};

}// namespace sys

}// namespace r
