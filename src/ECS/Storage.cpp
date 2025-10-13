#include <R-Engine/ECS/Storage.hpp>

namespace r::ecs {

/** --- Table --- */

usize Table::add_entity(Entity e)
{
    entities.push_back(e);
    return entities.size() - 1;
}

Entity Table::remove_entity_swap_back(usize row)
{
    Entity swapped_entity = 0;
    [[maybe_unused]] Entity removed_entity = entities[row];

    if (row < entities.size() - 1) {
        std::swap(entities[row], entities.back());
        swapped_entity = entities[row];
    }
    entities.pop_back();

    for (auto &col : columns) {
        col->remove_swap_back(row);
    }
    return swapped_entity;
}

/** --- Archetype --- */

bool Archetype::has_component(const std::type_index &type) const
{
    return component_map.count(type);
}

}// namespace r::ecs
