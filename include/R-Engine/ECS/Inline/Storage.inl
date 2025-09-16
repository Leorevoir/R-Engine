#pragma once

/**
 * Storage Template Implementations
 */

/**
 * @brief Get unique type ID for type T
 */
template<typename T>
static inline u64 r::ecs::type_id() noexcept
{
    static const u64 id = g_type_counter.fetch_add(1u, std::memory_order_relaxed);

    return id;
}

/**
 * @brief Add a component of type T to an entity
 * @param e The entity
 * @param comp The component to add (moved)
 */
template<typename T>
void r::ecs::ComponentStorage<T>::add(Entity e, T comp) noexcept
{
    _data[e] = std::make_unique<T>(std::move(comp));
}

/**
 * @brief Get a pointer to the component of type T for an entity
 * @param e The entity
 * @return Pointer to component, or nullptr if not found
 */
template<typename T>
T *r::ecs::ComponentStorage<T>::get_ptr(Entity e) noexcept
{
    const auto it = _data.find(e);
    return (it == _data.end()) ? nullptr : it->second.get();
}

/**
 * @brief Remove the component of type T for an entity
 * @param e The entity
 */
template<typename T>
void r::ecs::ComponentStorage<T>::remove(Entity e) noexcept
{
    _data.erase(e);
}

/**
 * @brief Get a list of all entities with this component
 * @return Vector of entities
 */
template<typename T>
std::vector<r::ecs::Entity> r::ecs::ComponentStorage<T>::entity_list() const noexcept
{
    std::vector<Entity> out;
    out.reserve(_data.size());
    for (const auto &d : _data) {
        out.push_back(d.first);
    }
    return out;
}

/**
 * @brief Check if an entity has this component
 * @param e The entity
 * @return true if component exists, false otherwise
 */
template<typename T>
bool r::ecs::ComponentStorage<T>::has(Entity e) const noexcept
{
    return _data.find(e) != _data.end();
}
