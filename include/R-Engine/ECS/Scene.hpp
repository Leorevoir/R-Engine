#pragma once

#include <R-Engine/ECS/Storage.hpp>
#include <R-Engine/Types.hpp>

#include <any>
#include <unordered_map>

namespace r {

namespace ecs {

struct Commands;

class Scene : public NonCopyable
{
    public:
        Scene() = default;
        ~Scene() = default;

        using StorageMap = std::unordered_map<u64, StoragePtr>;
        using ResourceMap = std::unordered_map<u64, std::any>;

        template<typename T>
        ComponentStorage<T> &storage()
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
        void add_component(Entity e, T comp)
        {
            storage<T>().add(e, std::move(comp));
        }

        template<typename T>
        T *get_component_ptr(Entity e)
        {
            const u64 id = type_id<T>();
            const auto it = _storages.find(id);
            if (it == _storages.end()) {
                return nullptr;
            }
            return static_cast<ComponentStorage<T> *>(it->second.get())->get_ptr(e);
        }

        template<typename T>
        bool has_component(Entity e) const
        {
            const u64 id = type_id<T>();
            const auto it = _storages.find(id);
            if (it == _storages.end()) {
                return false;
            }
            return it->second->has(e);
        }

        template<typename T>
        void insert_resource(T r)
        {
            _resources[type_id<T>()] = std::move(r);
        }

        template<typename T>
        T *get_resource_ptr()
        {
            const auto it = _resources.find(type_id<T>());

            if (it == _resources.end()) {
                return nullptr;
            }
            return std::any_cast<T>(&it->second);
        }

        StorageMap &getStorages();
        Commands make_commands();
        Entity create_entity();
        void destroy_entity(Entity e);

    private:
        StorageMap _storages;
        ResourceMap _resources;

        Entity _next_entity = 1;
};

}// namespace ecs

}// namespace r
