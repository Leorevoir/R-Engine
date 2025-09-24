#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>

#include <algorithm>
#include <any>
#include <atomic>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace r::ecs {

/**
 * @brief Global atomic counter to generate unique type IDs.
 * @note This is not central to the archetype-based design but can be useful for other purposes.
 */
inline std::atomic<u64> g_type_counter{0u};

/**
 * @brief Generates a unique, static ID for a given type T.
 * @tparam T The type to get an ID for.
 * @return A unique 64-bit integer ID.
 */
template<typename T>
static inline u64 type_id() noexcept
{
    static const u64 id = g_type_counter.fetch_add(1u, std::memory_order_relaxed);
    return id;
}

/** @name Table Storage Structures */
///@{

/**
 * @brief Type-erased interface for a column of components (e.g., a std::vector<Position>).
 * @details This allows storing columns of different component types in a single collection.
 */
struct R_ENGINE_API IColumn {
        virtual ~IColumn() = default;

        /**
         * @brief Adds a type-erased component to the end of the column.
         * @param component The component to add, wrapped in std::any.
         */
        virtual void push_back(std::any component) = 0;

        /**
         * @brief Removes a component by swapping it with the last element and popping.
         * @param index The index of the component to remove.
         */
        virtual void remove_swap_back(usize index) = 0;

        /**
         * @brief Gets a raw pointer to a component at a given index.
         * @param index The index of the component.
         * @return A void pointer to the component data.
         */
        virtual void *get_ptr(usize index) = 0;

        /**
         * @brief Moves a component from this column to another column.
         * @param index The index of the component to move in the source column.
         * @param dest The destination column.
         */
        virtual void move_to(usize index, IColumn &dest) = 0;

        /**
         * @brief Creates a new, empty column of the same underlying type.
         * @return A unique_ptr to the new IColumn.
         */
        virtual std::unique_ptr<IColumn> clone_empty() const = 0;
};

/**
 * @brief A typed implementation of a component column, using a std::vector for storage.
 * @tparam T The component type.
 */
template<typename T>
struct Column : IColumn {
        std::vector<T> data;

        void push_back(std::any component) override;
        void remove_swap_back(usize index) override;
        void *get_ptr(usize index) override;
        void move_to(usize index, IColumn &dest) override;
        std::unique_ptr<IColumn> clone_empty() const override;
};

/**
 * @brief Stores the actual component data for an Archetype.
 * @details Contains a vector of entities and a corresponding vector of component columns.
 */
struct R_ENGINE_API Table {
        std::vector<Entity> entities;
        std::vector<std::unique_ptr<IColumn>> columns;

        /**
         * @brief Adds an entity to the table.
         * @param e The entity to add.
         * @return The new row index for the entity.
         */
        usize add_entity(Entity e);
        /**
         * @brief Removes an entity from the table using swap-and-pop.
         * @param row The row index of the entity to remove.
         * @return The ID of the entity that was swapped into the removed row, or 0 if no swap occurred.
         */
        Entity remove_entity_swap_back(usize row);
};

/**
 * @brief Represents a unique combination of component types.
 * @details An archetype groups all entities that have the exact same set of components.
 */
struct R_ENGINE_API Archetype {
        std::vector<std::type_index> component_types;
        std::unordered_map<std::type_index, usize> component_map; /**< Maps component type to column index in the table. */
        Table table;

        /** @brief Caching for archetype transitions. */
        ///@{
        std::unordered_map<std::type_index, usize> add_edge;
        std::unordered_map<std::type_index, usize> remove_edge;
        ///@}

        /**
         * @brief Checks if this archetype contains a specific component type.
         * @param type The type_index of the component.
         * @return True if the component is part of the archetype, false otherwise.
         */
        bool has_component(const std::type_index &type) const;
};

///@}

}// namespace r::ecs

#include "Inline/Storage.inl"
