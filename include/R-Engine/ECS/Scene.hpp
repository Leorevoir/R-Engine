#pragma once

#include <R-Engine/ECS/Storage.hpp>
#include <R-Engine/Types.hpp>

#include <any>
#include <unordered_map>

namespace r {

namespace ecs {

struct Commands;

/**
* @brief Scene class that manages entities, components, and resources.
* @info create & destroy entities, add & get components, insert & get resources.
*/
class Scene : public NonCopyable
{
    public:
        Scene() = default;
        ~Scene() = default;

        using StorageMap = std::unordered_map<u64, StoragePtr>;
        using ResourceMap = std::unordered_map<u64, std::any>;

        /**
        * @brief get the storage for a specific component type T.
        * If the storage does not exist, it will be created.
        */
        template<typename T>
        ComponentStorage<T> &storage();

        /**
        * @brief Add a component of type T to an entity.
        * @param e The entity to which the component will be added.
        * @param comp The component to be added.
        */
        template<typename T>
        void add_component(Entity e, T comp);

        /**
        * @brief Get a pointer to the component of type T associated with an entity.
        * @param e The entity whose component is to be retrieved.
        * @return A pointer to the component of type T, or nullptr if the entity does
        */
        template<typename T>
        T *get_component_ptr(Entity e);

        /**
        * @brief Check if an entity has a component of type T.
        * @param e The entity to check.
        * @return true if the entity has the component of type T, false otherwise.
        */
        template<typename T>
        bool has_component(Entity e) const;

        /**
         * @brief Insert a resource of type T into the scene.
         * @param r The resource to be inserted.
         */
        template<typename T>
        void insert_resource(T r);

        /**
        * @brief Get a pointer to a resource of type T.
        * @return A pointer to the resource of type T, or nullptr if the resource does not exist.
        */
        template<typename T>
        T *get_resource_ptr();

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

#include "Inline/Scene.inl"
