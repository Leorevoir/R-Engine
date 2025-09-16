#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/Types.hpp>

#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>

namespace r {

namespace ecs {

/**
* @brief generate unique type IDs at runtime used for component type identification
* @info atomic because Scene::storage<T> can be called from multiple threads
*/
static inline u64 next_type_id()
{
    static std::atomic<u64> c{0};
    return c++;
}

/**
* @brief get unique type ID for type T
* @info each type T will get its own unique
*/
template<typename T>
static inline u64 type_id()
{
    static u64 id = next_type_id();

    return id;
}

/**
* @brief interface for component storage
* @info used for type-erased storage in Scene
*/
struct IComponentStorage {
        virtual ~IComponentStorage() = default;

        virtual void remove(Entity) = 0;
        virtual std::vector<Entity> entity_list() const = 0;
        virtual bool has(Entity) const = 0;
};

/**
* @brief type-erased pointer to component storage
* @info used in Scene
*/
using StoragePtr = std::unique_ptr<IComponentStorage>;

/**
* @brief typed component storage
* @info stores components of type T mapped to Entities
*/
template<typename T>
struct ComponentStorage : IComponentStorage {

    public:
        void add(Entity e, T comp)
        {
            _data[e] = std::make_unique<T>(std::move(comp));
        }

        T *get_ptr(Entity e)
        {
            const auto it = _data.find(e);

            return (it == _data.end()) ? nullptr : it->second.get();
        }

        void remove(Entity e) override
        {
            _data.erase(e);
        }

        std::vector<Entity> entity_list() const override
        {
            std::vector<Entity> out;

            out.reserve(_data.size());
            for (const auto &d : _data) {
                out.push_back(d.first);
            }
            return out;
        }

        bool has(Entity e) const override
        {
            return _data.find(e) != _data.end();
        }

    private:
        std::unordered_map<Entity, std::unique_ptr<T>> _data;
};

}// namespace ecs

}// namespace r
