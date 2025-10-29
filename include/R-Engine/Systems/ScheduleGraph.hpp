#pragma once

#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Scene.hpp>

#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace r {

namespace sys {

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

struct Access {
        std::unordered_set<std::type_index> reads;
        std::unordered_set<std::type_index> writes;
};

struct R_ENGINE_API SystemNode {
        SystemNode();
        SystemNode(const std::string &p_name, SystemTypeId p_id, SystemFn p_func, std::vector<SystemTypeId> p_dependencies);

        std::string name;
        SystemTypeId id;
        SystemFn func = nullptr;
        std::vector<SystemTypeId> dependencies;
        std::function<bool(ecs::Scene &)> condition = nullptr;
        std::vector<SystemSetId> member_of_sets;
        std::vector<SystemSetId> after_sets;
        std::vector<SystemSetId> before_sets;
        Access component_access;
        Access resource_access;
        bool is_main_thread_only = false;
};

/**
 * @brief Represents a named group of systems.
 * @details Systems can belong to multiple sets. Sets can have ordering constraints
 * relative to other sets using before_sets and after_sets.
 */
struct R_ENGINE_API SystemSet {
        SystemSet(const std::string &pname, SystemSetId pid) noexcept;

        std::string name;
        SystemSetId id;
        std::vector<SystemSetId> before_sets;
        std::vector<SystemSetId> after_sets;
};

struct ScheduleGraph {
        std::unordered_map<SystemTypeId, SystemNode> nodes;
        std::unordered_map<SystemSetId, SystemSet> sets;
        std::vector<std::vector<const SystemNode *>> execution_stages;
        bool dirty = true;
};

}// namespace sys

}// namespace r
