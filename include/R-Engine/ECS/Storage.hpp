#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/Types.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

namespace r {

namespace ecs {

/**
 * @brief generate unique type IDs at runtime
 * @info atomic because Scene::storage<T> can be called from multiple threads
 */
static inline u64 next_type_id();

/**
 * @brief get unique type ID for type T
 * @info each type T will get its own unique
 */
template<typename T>
static inline u64 type_id();

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
        void add(Entity e, T comp);
        T *get_ptr(Entity e);

        void remove(Entity e) override;
        std::vector<Entity> entity_list() const override;
        bool has(Entity e) const override;

    private:
        std::unordered_map<Entity, std::unique_ptr<T>> _data;
};

}// namespace ecs

}// namespace r

#include "Inline/Storage.inl"
