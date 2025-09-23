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

        auto arch_it = _archetype_map.find(new_types);
        if (arch_it != _archetype_map.end()) {
            new_archetype_idx = arch_it->second;
        } else {
            new_archetype_idx = _archetypes.size();
            auto &new_arch = _archetypes.emplace_back();
            new_arch.component_types = new_types;
            for (size_t i = 0; i < new_types.size(); ++i) {
                new_arch.component_map[new_types[i]] = i;
            }
            _archetype_map[new_types] = new_archetype_idx;
        }
        /** Get a fresh reference after potential emplace_back */
        _archetypes[old_archetype_idx].add_edge[comp_type] = new_archetype_idx;
        _archetypes[new_archetype_idx].remove_edge[comp_type] = old_archetype_idx;
    }

    /** --- Get fresh references now that the vector is stable --- */
    Archetype &old_archetype = _archetypes[old_archetype_idx];
    Archetype &new_archetype = _archetypes[new_archetype_idx];
    Table &old_table = old_archetype.table;
    Table &new_table = new_archetype.table;

    /** --- Move The Entity and its Components --- */
    const usize new_row = new_table.add_entity(e);
    if (new_table.columns.empty()) {
        new_table.columns.resize(new_archetype.component_types.size());
    }

    /** Move existing components */
    for (size_t i = 0; i < old_archetype.component_types.size(); ++i) {
        const auto &type_idx = old_archetype.component_types[i];
        const usize new_col_idx = new_archetype.component_map.at(type_idx);
        if (!new_table.columns[new_col_idx]) {
            new_table.columns[new_col_idx] = old_table.columns[i]->clone_empty();
        }
        old_table.columns[i]->move_to(loc.table_row, *new_table.columns[new_col_idx]);
    }

    /** Add the new component */
    const usize new_comp_col_idx = new_archetype.component_map.at(comp_type);
    if (!new_table.columns[new_comp_col_idx]) {
        new_table.columns[new_comp_col_idx] = std::make_unique<Column<T>>();
    }
    new_table.columns[new_comp_col_idx]->push_back(std::move(comp));

    /** Clean up the old table */
    Entity swapped_entity = old_table.remove_entity_swap_back(loc.table_row);

    /** Update entity locations */
    loc.archetype_index = new_archetype_idx;
    loc.table_row = new_row;
    if (swapped_entity != 0) {
        _entity_locations.at(swapped_entity).table_row = loc.table_row;
    }
}

template<typename T>
void r::ecs::Scene::remove_component(Entity e)
{
    auto loc_it = _entity_locations.find(e);
    if (loc_it == _entity_locations.end()) return;

    auto &loc = loc_it->second;
    const usize old_archetype_idx = loc.archetype_index;
    const std::type_index comp_type = typeid(T);

    if (!_archetypes[old_archetype_idx].has_component(comp_type)) return;

    /** --- Find or Create Destination Archetype (this may reallocate _archetypes) --- */
    usize new_archetype_idx;
    auto edge_it = _archetypes[old_archetype_idx].remove_edge.find(comp_type);

    if (edge_it != _archetypes[old_archetype_idx].remove_edge.end()) {
        new_archetype_idx = edge_it->second;
    } else {
        auto new_types = _archetypes[old_archetype_idx].component_types;
        new_types.erase(std::remove(new_types.begin(), new_types.end(), comp_type), new_types.end());

        auto arch_it = _archetype_map.find(new_types);
        if (arch_it != _archetype_map.end()) {
            new_archetype_idx = arch_it->second;
        } else {
            new_archetype_idx = _archetypes.size();
            auto &new_arch = _archetypes.emplace_back();
            new_arch.component_types = new_types;
            for (size_t i = 0; i < new_types.size(); ++i) {
                new_arch.component_map[new_types[i]] = i;
            }
            _archetype_map[new_types] = new_archetype_idx;
        }

        /** Get fresh references after potential emplace_back */
        _archetypes[old_archetype_idx].remove_edge[comp_type] = new_archetype_idx;
        _archetypes[new_archetype_idx].add_edge[comp_type] = old_archetype_idx;
    }

    /** --- Get fresh references now that the vector is stable --- */
    Archetype &old_archetype = _archetypes[old_archetype_idx];
    Archetype &new_archetype = _archetypes[new_archetype_idx];
    Table &old_table = old_archetype.table;
    Table &new_table = new_archetype.table;

    /** --- Move The Entity and its Components --- */
    const usize new_row = new_table.add_entity(e);
    if (new_table.columns.empty()) {
        new_table.columns.resize(new_archetype.component_types.size());
    }

    for (size_t i = 0; i < old_archetype.component_types.size(); ++i) {
        const auto &type_idx = old_archetype.component_types[i];
        if (type_idx == comp_type) continue;
        const usize new_col_idx = new_archetype.component_map.at(type_idx);
        if (!new_table.columns[new_col_idx]) {
            new_table.columns[new_col_idx] = old_table.columns[i]->clone_empty();
        }
        old_table.columns[i]->move_to(loc.table_row, *new_table.columns[new_col_idx]);
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

template<typename T>
T *r::ecs::Scene::get_component_ptr(Entity e)
{
    const auto *loc = get_entity_location(e);
    if (!loc) return nullptr;

    const Archetype &archetype = _archetypes[loc->archetype_index];
    const std::type_index comp_type = typeid(T);

    auto comp_it = archetype.component_map.find(comp_type);
    if (comp_it == archetype.component_map.end()) return nullptr;

    const usize col_idx = comp_it->second;
    return static_cast<T *>(archetype.table.columns[col_idx]->get_ptr(loc->table_row));
}

template<typename T>
bool r::ecs::Scene::has_component(Entity e) const
{
    const auto *loc = get_entity_location(e);
    if (!loc) return false;

    const Archetype &archetype = _archetypes[loc->archetype_index];
    return archetype.has_component(typeid(T));
}

template<typename T>
void r::ecs::Scene::insert_resource(T r) noexcept
{
    _resources[std::type_index(typeid(T))] = std::move(r);
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
