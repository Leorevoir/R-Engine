#pragma once

/**
 * Scene Implementation (templates)
 */

template<typename T>
r::ecs::ComponentStorage<T> &r::ecs::Scene::storage()
{
    const u64 id = type_id<T>();
    const auto it = _storages.find(id);

    if (it == _storages.end()) {
        auto ptr = std::make_unique<ComponentStorage<T>>();
        ComponentStorage<T> *raw = ptr.get();
        _storages.emplace(id, std::move(ptr));
        return *raw;
    }

    return *static_cast<ComponentStorage<T> *>(_storages[id].get());
}

template<typename T>
void r::ecs::Scene::add_component(Entity e, T comp)
{
    storage<T>().add(e, std::move(comp));
}

template<typename T>
T *r::ecs::Scene::get_component_ptr(Entity e)
{
    const u64 id = type_id<T>();
    const auto it = _storages.find(id);
    if (it == _storages.end()) {
        return nullptr;
    }
    return static_cast<ComponentStorage<T> *>(it->second.get())->get_ptr(e);
}

template<typename T>
bool r::ecs::Scene::has_component(Entity e) const
{
    const u64 id = type_id<T>();
    const auto it = _storages.find(id);
    if (it == _storages.end()) {
        return false;
    }
    return it->second->has(e);
}

template<typename T>
void r::ecs::Scene::insert_resource(T r)
{
    _resources[type_id<T>()] = std::move(r);
}

template<typename T>
T *r::ecs::Scene::get_resource_ptr()
{
    const auto it = _resources.find(type_id<T>());

    if (it == _resources.end()) {
        return nullptr;
    }
    return std::any_cast<T>(&it->second);
}
