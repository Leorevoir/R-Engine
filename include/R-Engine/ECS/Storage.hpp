#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>

#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>

namespace r {

namespace ecs {

/**
 * @brief Global type id counter shared across translation units and shared libraries
 *
 * Why?
 * - 'inline' ensures a single definition across TUs (C++17).
 * - using atomic and fetch_add makes it safe for concurrent initialization.
 */
inline std::atomic<u64> g_type_counter{0u};

/**
 * @brief get unique type ID for type T
 * @info each type T will get its own unique
 */
template<typename T>
static inline u64 type_id() noexcept;

/**
 * @brief interface for component storage
 * @info used for type-erased storage in Scene
 */
struct R_ENGINE_API IComponentStorage {
        virtual ~IComponentStorage() = default;

        virtual void remove(Entity) noexcept = 0;
        virtual std::vector<Entity> entity_list() const noexcept = 0;
        virtual bool has(Entity) const noexcept = 0;
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
struct R_ENGINE_API ComponentStorage : IComponentStorage {
    public:
        void add(Entity e, T comp) noexcept;
        T *get_ptr(Entity e) noexcept;

        void remove(Entity e) noexcept override;
        std::vector<Entity> entity_list() const noexcept override;
        bool has(Entity e) const noexcept override;

    private:
        std::unordered_map<Entity, std::unique_ptr<T>> _data;
};

}// namespace ecs

}// namespace r

#include "Inline/Storage.inl"
