#include <R-Engine/Systems/ScheduleGraph.hpp>

/**
 * public
 */

r::sys::SystemNode::SystemNode() : id(typeid(void)), func(nullptr)
{
    /* __ctor__ */
}

r::sys::SystemNode::SystemNode(const std::string &p_name, SystemTypeId p_id, SystemFn p_func, std::vector<SystemTypeId> p_dependencies)
    : name(std::move(p_name)), id(p_id), func(p_func), dependencies(std::move(p_dependencies))
{
    /* __ctor__ */
}

r::sys::SystemSet::SystemSet(const std::string &pname, SystemSetId pid) noexcept : name(pname), id(pid)
{
    /* __ctor__ */
}
