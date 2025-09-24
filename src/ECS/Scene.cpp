#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Scene.hpp>

r::ecs::Scene::Scene()
{
    /** Create the initial empty archetype at index 0 */
    _archetypes.emplace_back();
    _archetype_map[{}] = 0;
}

const std::vector<r::ecs::Archetype> &r::ecs::Scene::get_archetypes() const
{
    return _archetypes;
}

const r::ecs::EntityLocation *r::ecs::Scene::get_entity_location(r::ecs::Entity e) const
{
    auto it = _entity_locations.find(e);
    if (it == _entity_locations.end()) {
        return nullptr;
    }
    return &it->second;
}

r::ecs::Entity r::ecs::Scene::create_entity()
{
    const Entity new_entity = _next_entity++;
    Archetype &empty_archetype = _archetypes[0];
    const usize row = empty_archetype.table.add_entity(new_entity);
    _entity_locations[new_entity] = {0, row};
    return new_entity;
}

void r::ecs::Scene::destroy_entity(r::ecs::Entity e) noexcept
{
    const auto it = _entity_locations.find(e);
    if (it == _entity_locations.end()) {
        return;
    }

    const EntityLocation loc = it->second;
    Archetype &archetype = _archetypes[loc.archetype_index];

    Entity swapped_entity = archetype.table.remove_entity_swap_back(loc.table_row);

    _entity_locations.erase(e);
    if (swapped_entity != 0) {
        _entity_locations[swapped_entity].table_row = loc.table_row;
    }
}

void r::ecs::Scene::clear_command_buffer_placeholder_map() noexcept
{
    _placeholder_map.clear();
}

void r::ecs::Scene::map_command_buffer_placeholder(Entity placeholder, Entity real) noexcept
{
    _placeholder_map[placeholder] = real;
}

const std::unordered_map<r::ecs::Entity, r::ecs::Entity> &r::ecs::Scene::get_command_buffer_placeholder_map() const noexcept
{
    return _placeholder_map;
}
