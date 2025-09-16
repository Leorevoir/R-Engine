#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Scene.hpp>

/**
* public
*/

r::ecs::Scene::StorageMap &r::ecs::Scene::getStorages()
{
    return _storages;
}

r::ecs::Commands r::ecs::Scene::make_commands()
{
    return Commands(this);
}

r::ecs::Entity r::ecs::Scene::create_entity()
{
    return _next_entity++;
}

void r::ecs::Scene::destroy_entity(r::ecs::Entity e)
{
    for (const auto &kv : _storages) {
        kv.second->remove(e);
    }
}
