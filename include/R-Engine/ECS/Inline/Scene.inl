#pragma once

#include "R-Engine/ECS/Scene.hpp"
#include <algorithm>

template<typename T>
void r::ecs::Scene::add_component(Entity e, T comp)
{
    auto loc_it = _entity_locations.find(e);
    if (loc_it == _entity_locations.end())
        return;

    auto &loc = loc_it->second;
    const usize old_archetype_idx = loc.archetype_index;
    const std::type_index comp_type = typeid(T);

    /** If the component already exists, just update it. */
    if (_archetypes[old_archetype_idx].has_component(comp_type)) {
        Archetype &arch = _archetypes[old_archetype_idx];
        *static_cast<T *>(arch.table.columns[arch.component_map.at(comp_type)]->get_ptr(loc.table_row)) = std::move(comp);
        return;
    }

    /** --- Find or Create Destination Archetype (this may reallocate _archetypes) --- */
    usize new_archetype_idx;
    auto edge_it = _archetypes[old_archetype_idx].add_edge.find(comp_type);

    if (edge_it != _archetypes[old_archetype_idx].add_edge.end()) {
        new_archetype_idx = edge_it->second;
    } else {
        auto new_types = _archetypes[old_archetype_idx].component_types;
        new_types.push_back(comp_type);
        std::sort(new_types.begin(), new_types.end());
        new_archetype_idx = _find_or_create_archetype(new_types);
        _archetypes[old_archetype_idx].add_edge[comp_type] = new_archetype_idx;
        _archetypes[new_archetype_idx].remove_edge[comp_type] = old_archetype_idx;
    }

    /** --- Move entity and common components to the new archetype --- */
    _move_entity_between_archetypes(e, loc, new_archetype_idx);

    /** --- Add the new component to the new location --- */
    Archetype &new_archetype = _archetypes[new_archetype_idx];
    const usize new_comp_col_idx = new_archetype.component_map.at(comp_type);
    if (!new_archetype.table.columns[new_comp_col_idx]) {
        new_archetype.table.columns[new_comp_col_idx] = std::make_unique<Column<T>>();
    }
    new_archetype.table.columns[new_comp_col_idx]->push_back(std::move(comp));
}

template<typename T>
void r::ecs::Scene::remove_component(Entity e)
{
    auto loc_it = _entity_locations.find(e);
    if (loc_it == _entity_locations.end())
        return;

    auto &loc = loc_it->second;
    const usize old_archetype_idx = loc.archetype_index;
    const std::type_index comp_type = typeid(T);

    if (!_archetypes[old_archetype_idx].has_component(comp_type))
        return;

    /** --- Find or Create Destination Archetype (this may reallocate _archetypes) --- */
    usize new_archetype_idx;
    auto edge_it = _archetypes[old_archetype_idx].remove_edge.find(comp_type);

    if (edge_it != _archetypes[old_archetype_idx].remove_edge.end()) {
        new_archetype_idx = edge_it->second;
    } else {
        auto new_types = _archetypes[old_archetype_idx].component_types;
        new_types.erase(std::remove(new_types.begin(), new_types.end(), comp_type), new_types.end());
        new_archetype_idx = _find_or_create_archetype(new_types);
        _archetypes[old_archetype_idx].remove_edge[comp_type] = new_archetype_idx;
        _archetypes[new_archetype_idx].add_edge[comp_type] = old_archetype_idx;
    }

    /** --- Move entity and common components to the new archetype --- */
    _move_entity_between_archetypes(e, loc, new_archetype_idx);
}

template<typename T>
T *r::ecs::Scene::get_component_ptr(Entity e)
{
    const auto *loc = get_entity_location(e);

    if (!loc) {
        return nullptr;
    }

    const Archetype &archetype = _archetypes[loc->archetype_index];
    const std::type_index comp_type = typeid(T);

    const auto comp_it = archetype.component_map.find(comp_type);

    if (comp_it == archetype.component_map.end()) {
        return nullptr;
    }

    const usize col_idx = comp_it->second;
    return static_cast<T *>(archetype.table.columns[col_idx]->get_ptr(loc->table_row));
}

template<typename T>
bool r::ecs::Scene::has_component(Entity e) const
{
    const auto *loc = get_entity_location(e);

    if (!loc) {
        return false;
    }

    const Archetype &archetype = _archetypes[loc->archetype_index];
    return archetype.has_component(typeid(T));
}

template<typename T>
void r::ecs::Scene::insert_resource(T r) noexcept
{
    _resources[std::type_index(typeid(T))] = std::move(r);
}

template<typename T>
void r::ecs::Scene::remove_resource() noexcept
{
    _resources.erase(std::type_index(typeid(T)));
}

template<typename T>
T *r::ecs::Scene::get_resource_ptr() noexcept
{
    const auto it = _resources.find(std::type_index(typeid(T)));

    if (it == _resources.end()) {
        return nullptr;
    }
    return std::any_cast<T>(&it->second);
}
