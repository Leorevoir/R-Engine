#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/ECS/Scene.hpp>
#include <functional>
#include <limits>
#include <vector>

namespace r::ecs {

class CommandBuffer;
struct Commands;

/**
 * @brief A handle for creating commands for a specific entity.
 * @details Returned by `Commands::spawn()` and `Commands::entity()`.
 * Allows for a chained builder pattern, e.g. `commands.spawn().insert(Pos{}).insert(Vel{});`
 */
class EntityCommands
{
    public:
        EntityCommands(CommandBuffer *buffer, Entity entity) noexcept;

        /**
         * @brief Schedules a component of type T to be added to the entity.
         * @param component The component data to insert.
         */
        template<typename T>
        EntityCommands &insert(T component) noexcept;

        /**
         * @brief Schedules a component of type T to be removed from the entity.
         */
        template<typename T>
        EntityCommands &remove() noexcept;

        /**
         * @brief Returns the entity's ID.
         * @details If the entity was just spawned, this is a temporary placeholder ID
         * that is only valid within this command buffer cycle.
         */
        Entity id() const noexcept;

    private:
        CommandBuffer *_buffer;
        Entity _entity;
};

/**
 * @brief A buffer that stores commands to be applied to the Scene later.
 * @details This decouples structural ECS changes (spawning, despawning, adding/removing components)
 * from system execution, preventing iterator invalidation and other state-related bugs.
 */
class CommandBuffer
{
    public:
        CommandBuffer() = default;

        /**
         * @brief Applies all buffered commands to the scene and clears the buffer.
         * @param scene The scene to apply commands to.
         */
        void apply(Scene &scene);

        /**
         * @brief Schedules a command to add a component to an entity.
         */
        template<typename T>
        void add_component(Entity e, T component);

        /**
         * @brief Schedules a command to remove a component from an entity.
         */
        template<typename T>
        void remove_component(Entity e);

        /**
         * @brief Schedules a command to despawn an entity.
         */
        void despawn(Entity e);

        /**
         * @brief Generates a placeholder entity and schedules its creation.
         */
        Entity spawn_entity();

    private:
        friend struct Commands;

        void add_command(std::function<void(Scene &)> &&command);

        std::vector<std::function<void(Scene &)>> _commands;
        Entity _next_placeholder = std::numeric_limits<Entity>::max();
};

/**
 * @brief Main interface for systems to issue commands to modify the Scene.
 * @details This is a system parameter that can be requested by any system function.
 * All operations are deferred and applied at the end of the current schedule.
 */
struct Commands {
    public:
        explicit Commands(CommandBuffer *buffer = nullptr) noexcept;

        /**
         * @brief Schedules an entity to be spawned with no initial components.
         * @return An EntityCommands handle to chain further commands like `insert`.
         */
        EntityCommands spawn() noexcept;

        /**
         * @brief Schedules an entity to be spawned with a set of initial components.
         * @details This is the preferred way to create entities with their starting components.
         * @param components The components to insert into the new entity.
         * @return An EntityCommands handle to chain further commands.
         */
        template<typename... Components>
        EntityCommands spawn(Components &&...components) noexcept;

        /**
         * @brief Returns an EntityCommands handle for an existing entity.
         * @param e The ID of the existing entity.
         */
        EntityCommands entity(Entity e) noexcept;

        /**
         * @brief Schedules a component to be added to an entity.
         */
        template<typename T>
        void add_component(Entity e, T comp) noexcept;

        /**
         * @brief Schedules an entity and all its components to be despawned.
         */
        void despawn(Entity e) noexcept;

    private:
        CommandBuffer *_buffer;
};

}// namespace r::ecs

#include "Inline/Command.inl"
