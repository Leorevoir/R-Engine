#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Scene.hpp>

/**
* public
*/

r::ecs::Scene::StorageMap &r::ecs::Scene::get_storages() noexcept
{
    return _storages;
}

r::ecs::Entity r::ecs::Scene::create_entity() noexcept
{
    return _next_entity++;
}

void r::ecs::Scene::destroy_entity(r::ecs::Entity e) noexcept
{
    for (const auto &kv : _storages) {
        kv.second->remove(e);
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
