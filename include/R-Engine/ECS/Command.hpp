#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/ECS/Scene.hpp>

#include <functional>
#include <limits>
#include <vector>

namespace r::ecs {

class CommandBuffer;
struct Commands;
class EntityCommands;

struct Children;
struct Parent;

/**
 * @brief Builder for spawning child entities.
 */
class R_ENGINE_API ChildBuilder
{
    public:
        explicit ChildBuilder(Commands *commands, Entity parent) noexcept;

        /**
         * @brief Spawn a child entity with components
         */
        template<typename... Components>
        EntityCommands spawn(Components &&...components) noexcept;

    private:
        Commands *_commands;
        Entity _parent;
};

/**
 * @brief A handle for creating commands for a specific entity.
 * @details Returned by `Commands::spawn()` and `Commands::entity()`.
 * Allows for a chained builder pattern, e.g. `commands.spawn().insert(Pos{}).insert(Vel{});`
 */
class R_ENGINE_API EntityCommands
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

        /**
         * @brief Spawn child entities for this entity.
         * @param builder_fn Function that receives a ChildBuilder to spawn children.
         */
        template<typename FuncT>
        EntityCommands &with_children(FuncT &&func) noexcept;

    private:
        CommandBuffer *_buffer;
        Entity _entity;
};

/**
 * @brief A buffer that stores commands to be applied to the Scene later.
 * @details This decouples structural ECS changes (spawning, despawning, adding/removing components)
 * from system execution, preventing iterator invalidation and other state-related bugs.
 */
class R_ENGINE_API CommandBuffer
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

        /**
         * @brief Schedules adding a child to a parent's Children component.
         */
        void add_child(Entity parent, Entity child);

        /**
         * @brief Returns a Commands interface for this buffer.
         */
        Commands *get_commands() noexcept;

    private:
        friend struct Commands;

        void _add_command(std::function<void(Scene &)> &&command);

        std::vector<std::function<void(Scene &)>> _commands;
        Entity _next_placeholder = std::numeric_limits<Entity>::max();
        Commands *_commands_wrapper = nullptr;
};

/**
 * @brief Main interface for systems to issue commands to modify the Scene.
 * @details This is a system parameter that can be requested by any system function.
 * All operations are deferred and applied at the end of the current schedule.
 */
struct R_ENGINE_API Commands {
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

        /**
         * @brief Internal: Add a child to a parent's Children component.
         */
        void add_child(Entity parent, Entity child) noexcept;

    private:
        CommandBuffer *_buffer;
};

/**
 * @brief Component that marks an entity as having children.
 */
struct R_ENGINE_API Children final {
        std::vector<Entity> entities;
};

/**
 * @brief Component that marks an entity as having a parent.
 */
struct R_ENGINE_API Parent final {
        Entity entity = NULL_ENTITY;
};

}// namespace r::ecs

#include "Inline/Command.inl"
