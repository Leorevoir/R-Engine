#pragma once

#include <R-Engine/ECS/Storage.hpp>
#include <R-Engine/Types.hpp>

#include <any>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace r {

namespace ecs {

/**
 * @brief Hasher for a vector of type_index, needed for the archetype map.
 */
struct TypeVecHasher {
        std::size_t operator()(const std::vector<std::type_index> &vec) const
        {
            std::size_t seed = vec.size();
            for (const auto &i : vec) {
                seed ^= i.hash_code() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
};

/**
 * @brief Location of an entity within the ECS storage.
 */
struct EntityLocation {
        usize archetype_index;
        usize table_row;
};

/**
* @brief Scene class that manages entities, components, and resources.
* @details The Scene is the central container for all game state in the ECS. It holds all entities,
* their components, and global resources.
*/
class R_ENGINE_API Scene : public NonCopyable
{
    public:
        /**
         * @brief Constructs a new Scene, creating the initial empty archetype.
         */
        Scene();
        ~Scene() = default;

        using ResourceMap = std::unordered_map<std::type_index, std::any>;

        /**
         * @brief Adds a component of type T to an entity.
         * @details If the entity already has this component, it will be updated.
         * This operation may move the entity to a different archetype.
         * @tparam T The type of component to add.
         * @param e The entity to modify.
         * @param comp The component data to add.
         */
        template<typename T>
        void add_component(Entity e, T comp);

        /**
         * @brief Removes a component of type T from an entity.
         * @details This operation may move the entity to a different archetype.
         * @tparam T The type of component to remove.
         * @param e The entity to modify.
         */
        template<typename T>
        void remove_component(Entity e);

        /**
         * @brief Gets a pointer to an entity's component of type T.
         * @tparam T The type of component to get.
         * @param e The entity to query.
         * @return A pointer to the component, or nullptr if the entity does not have it.
         */
        template<typename T>
        T *get_component_ptr(Entity e);

        /**
         * @brief Checks if an entity has a component of type T.
         * @tparam T The type of component to check for.
         * @param e The entity to query.
         * @return True if the entity has the component, false otherwise.
         */
        template<typename T>
        bool has_component(Entity e) const;

        /**
         * @brief Inserts a resource into the scene.
         * @details Resources are unique, global data structures. If a resource of this
         * type already exists, it will be overwritten.
         * @tparam T The type of the resource.
         * @param r The resource to insert.
         */
        template<typename T>
        void insert_resource(T r) noexcept;

        /**
         * @brief Gets a pointer to a resource of type T.
         * @tparam T The type of the resource to get.
         * @return A pointer to the resource, or nullptr if it doesn't exist.
         */
        template<typename T>
        T *get_resource_ptr() noexcept;

        /**
         * @brief Creates a new entity with no components.
         * @return The ID of the newly created entity.
         */
        Entity create_entity();
        /**
         * @brief Destroys an entity and all its components.
         * @param e The entity to destroy.
         */
        void destroy_entity(Entity e) noexcept;

        /** @name Internal methods for Querying and Commands */
        ///@{

        /**
         * @brief Gets all archetypes in the scene.
         * @return A const reference to the vector of archetypes.
         */
        const std::vector<Archetype> &get_archetypes() const;
        /**
         * @brief Gets the storage location of an entity.
         * @param e The entity to locate.
         * @return A const pointer to the entity's location, or nullptr if not found.
         */
        const EntityLocation *get_entity_location(Entity e) const;

        /**
         * @brief Clears the placeholder map used by the command buffer.
         */
        void clear_command_buffer_placeholder_map() noexcept;
        /**
         * @brief Maps a temporary placeholder entity ID to its real ID after creation.
         * @param placeholder The temporary ID used during command buffer recording.
         * @param real The final ID assigned by the scene.
         */
        void map_command_buffer_placeholder(Entity placeholder, Entity real) noexcept;
        /**
         * @brief Gets the placeholder-to-real-entity map.
         * @return A const reference to the map.
         */
        const std::unordered_map<Entity, Entity> &get_command_buffer_placeholder_map() const noexcept;

        ///@}

    private:
        std::vector<Archetype> _archetypes;
        std::unordered_map<std::vector<std::type_index>, usize, TypeVecHasher> _archetype_map;
        std::unordered_map<Entity, EntityLocation> _entity_locations;

        ResourceMap _resources;
        std::unordered_map<Entity, Entity> _placeholder_map;

        Entity _next_entity = 1;

        usize _find_or_create_archetype(const std::vector<std::type_index>& types);
        void _move_entity_between_archetypes(Entity e, EntityLocation& loc, usize new_archetype_idx);
};

}// namespace ecs

#include "Inline/Scene.inl"

}// namespace r
