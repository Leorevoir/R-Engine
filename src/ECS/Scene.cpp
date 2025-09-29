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
    const auto it = _entity_locations.find(e);
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

usize r::ecs::Scene::_find_or_create_archetype(const std::vector<std::type_index>& types)
{
    const auto arch_it = _archetype_map.find(types);
    if (arch_it != _archetype_map.end()) {
        return arch_it->second;
    }

    const usize new_archetype_idx = _archetypes.size();
    auto &new_arch = _archetypes.emplace_back();
    new_arch.component_types = types;
    for (usize i = 0; i < types.size(); ++i) {
        new_arch.component_map[types[i]] = i;
    }
    _archetype_map[types] = new_archetype_idx;
    return new_archetype_idx;
}

void r::ecs::Scene::_move_entity_between_archetypes(Entity e, EntityLocation& loc, usize new_archetype_idx)
{
    const usize old_archetype_idx = loc.archetype_index;
    Archetype &old_archetype = _archetypes[old_archetype_idx];
    Archetype &new_archetype = _archetypes[new_archetype_idx];
    Table &old_table = old_archetype.table;
    Table &new_table = new_archetype.table;

    /** --- Move The Entity and its Common Components --- */
    const usize new_row = new_table.add_entity(e);
    if (new_table.columns.empty() && !new_archetype.component_types.empty()) {
        new_table.columns.resize(new_archetype.component_types.size());
    }

    // Iterate over the destination archetype's components and pull them from the source.
    for (const auto& [type_idx, new_col_idx] : new_archetype.component_map) {
        auto old_it = old_archetype.component_map.find(type_idx);
        if (old_it != old_archetype.component_map.end()) {
            const usize old_col_idx = old_it->second;
            if (!new_table.columns[new_col_idx]) {
                new_table.columns[new_col_idx] = old_table.columns[old_col_idx]->clone_empty();
            }
            old_table.columns[old_col_idx]->move_to(loc.table_row, *new_table.columns[new_col_idx]);
        }
    }

    /** Clean up the old table */
    Entity swapped_entity = old_table.remove_entity_swap_back(loc.table_row);

    /** Update entity locations */
    loc.archetype_index = new_archetype_idx;
    loc.table_row = new_row;
    if (swapped_entity != 0) {
        _entity_locations.at(swapped_entity).table_row = loc.table_row;
    }
}
