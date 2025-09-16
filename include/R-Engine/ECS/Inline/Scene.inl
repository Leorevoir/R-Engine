#pragma once

/**
* Scene Implementation
*/

template<typename T>
r::ecs::ComponentStorage<T> &r::ecs::Scene::storage() noexcept
{
    const auto key = std::type_index(typeid(T));
    const auto it = _storages.find(key);

    if (it == _storages.end()) {
        auto ptr = std::make_unique<ComponentStorage<T>>();
        ComponentStorage<T> *raw = ptr.get();

        _storages.emplace(key, std::move(ptr));
        return *raw;
    }
    return *static_cast<ComponentStorage<T> *>(_storages[key].get());
}

template<typename T>
void r::ecs::Scene::add_component(Entity e, T comp) noexcept
{
    storage<T>().add(e, std::move(comp));
}

template<typename T>
T *r::ecs::Scene::get_component_ptr(Entity e) noexcept
{
    const auto key = std::type_index(typeid(T));
    const auto it = _storages.find(key);

    if (it == _storages.end()) {
        return nullptr;
    }
    return static_cast<ComponentStorage<T> *>(it->second.get())->get_ptr(e);
}

template<typename T>
bool r::ecs::Scene::has_component(Entity e) const noexcept
{
    const auto key = std::type_index(typeid(T));
    const auto it = _storages.find(key);

    if (it == _storages.end()) {
        return false;
    }
    return it->second->has(e);
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
