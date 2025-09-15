#include <R-Engine/ECS/Entity.hpp>

/**
* public
*/

r::ecs::Entity::Entity(const u64 id, const bool active) : _id(id), _active(active)
{
    /* __ctor__ */
}

/**
 * getters
 */

u64 r::ecs::Entity::getID() const
{
    return _id;
}

bool r::ecs::Entity::isActive() const
{
    return _active;
}

const std::vector<std::unique_ptr<r::ecs::Component>> &r::ecs::Entity::getComponents() const
{
    return _components;
}

const r::core::BitMask &r::ecs::Entity::getEnabledComponents() const
{
    return _enabled_components;
}

/**
 * setters
 */

void r::ecs::Entity::setActive(const bool active)
{
    _active = active;
}

/**
* private
*/
