#pragma once

#include "R-Engine/ECS/Command.hpp"

/* EntityCommands */

inline r::ecs::EntityCommands::EntityCommands(r::ecs::CommandBuffer *buffer, Entity entity) noexcept : _buffer(buffer), _entity(entity)
{
}

template<typename T>
inline r::ecs::EntityCommands &r::ecs::EntityCommands::insert(T component) noexcept
{
    if (_buffer) {
        _buffer->add_component<T>(_entity, std::move(component));
    }
    return *this;
}

template<typename T>
inline r::ecs::EntityCommands &r::ecs::EntityCommands::remove() noexcept
{
    if (_buffer) {
        _buffer->remove_component<T>(_entity);
    }
    return *this;
}

inline r::ecs::Entity r::ecs::EntityCommands::id() const noexcept
{
    return _entity;
}

/* CommandBuffer */

inline void r::ecs::CommandBuffer::apply(Scene &scene)
{
    scene.clear_command_buffer_placeholder_map();
    for (const auto &command : _commands) {
        command(scene);
    }
    _commands.clear();
}

template<typename T>
inline void r::ecs::CommandBuffer::add_component(Entity e, T component)
{
    add_command([e, component = std::move(component)](Scene &scene) mutable {
        const auto &map = scene.get_command_buffer_placeholder_map();
        const auto it = map.find(e);
        const Entity real_entity = (it != map.end()) ? it->second : e;
        scene.add_component<T>(real_entity, std::move(component));
    });
}

template<typename T>
inline void r::ecs::CommandBuffer::remove_component(Entity e)
{
    add_command([e](Scene &scene) {
        const auto &map = scene.get_command_buffer_placeholder_map();
        const auto it = map.find(e);
        const Entity real_entity = (it != map.end()) ? it->second : e;
        scene.remove_component<T>(real_entity);
    });
}

inline void r::ecs::CommandBuffer::despawn(Entity e)
{
    add_command([e](Scene &scene) {
        const auto &map = scene.get_command_buffer_placeholder_map();
        const auto it = map.find(e);
        const Entity real_entity = (it != map.end()) ? it->second : e;
        scene.destroy_entity(real_entity);
    });
}

inline r::ecs::Entity r::ecs::CommandBuffer::spawn_entity()
{
    const Entity placeholder = _next_placeholder--;
    add_command([placeholder](Scene &scene) {
        const Entity real_entity = scene.create_entity();
        scene.map_command_buffer_placeholder(placeholder, real_entity);
    });
    return placeholder;
}

inline void r::ecs::CommandBuffer::add_command(std::function<void(Scene &)> &&command)
{
    _commands.emplace_back(std::move(command));
}

/* Commands */

inline r::ecs::Commands::Commands(CommandBuffer *buffer) noexcept : _buffer(buffer)
{
}

inline r::ecs::EntityCommands r::ecs::Commands::spawn() noexcept
{
    const Entity placeholder = _buffer ? _buffer->spawn_entity() : 0;
    return EntityCommands(_buffer, placeholder);
}

inline r::ecs::EntityCommands r::ecs::Commands::entity(Entity e) noexcept
{
    return EntityCommands(_buffer, e);
}

template<typename T>
void r::ecs::Commands::add_component(Entity e, T comp) noexcept
{
    if (_buffer) {
        _buffer->add_component<T>(e, std::move(comp));
    }
}

inline void r::ecs::Commands::despawn(Entity e) noexcept
{
    if (_buffer) {
        _buffer->despawn(e);
    }
}
