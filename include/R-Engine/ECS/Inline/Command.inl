#pragma once

#include "R-Engine/ECS/Command.hpp"

/**
 * EntityCommands
 */

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

template<typename FuncT>
inline r::ecs::EntityCommands &r::ecs::EntityCommands::with_children(FuncT &&func) noexcept
{
    if (_buffer) {
        ChildBuilder builder(_buffer->get_commands(), _entity);

        func(builder);
    }
    return *this;
}

/**
 * CommandBuffer
 */

template<typename T>
inline void r::ecs::CommandBuffer::add_component(Entity e, T component)
{
    _add_command([e, component = std::move(component)](Scene &scene) mutable {
        const auto &map = scene.get_command_buffer_placeholder_map();
        const auto it = map.find(e);
        const Entity real_entity = (it != map.end()) ? it->second : e;

        scene.add_component<T>(real_entity, std::move(component));
    });
}

template<typename T>
inline void r::ecs::CommandBuffer::remove_component(Entity e)
{
    _add_command([e](Scene &scene) {
        const auto &map = scene.get_command_buffer_placeholder_map();
        const auto it = map.find(e);
        const Entity real_entity = (it != map.end()) ? it->second : e;

        scene.remove_component<T>(real_entity);
    });
}

template<typename T>
inline void r::ecs::CommandBuffer::insert_resource(T resource)
{
    _add_command([res = std::move(resource)](Scene &scene) mutable { scene.insert_resource(std::move(res)); });
}

template<typename T>
inline void r::ecs::CommandBuffer::remove_resource()
{
    _add_command([](Scene &scene) { scene.remove_resource<T>(); });
}

/**
 * Commands
 */

template<typename... Components>
inline r::ecs::EntityCommands r::ecs::Commands::spawn(Components &&...components) noexcept
{
    EntityCommands entity_cmds = this->spawn();

    (entity_cmds.insert(std::forward<Components>(components)), ...);
    return entity_cmds;
}

template<typename T>
inline void r::ecs::Commands::insert_resource(T res) noexcept
{
    if (_buffer) {
        _buffer->insert_resource<T>(std::move(res));
    }
}

template<typename T>
inline void r::ecs::Commands::remove_resource() noexcept
{
    if (_buffer) {
        _buffer->remove_resource<T>();
    }
}

/**
 * ChildBuilder
 */

inline r::ecs::ChildBuilder::ChildBuilder(r::ecs::Commands *commands, r::ecs::Entity parent) noexcept : _commands(commands), _parent(parent)
{
    /* __ctor__ */
}

template<typename... Components>
inline r::ecs::EntityCommands r::ecs::ChildBuilder::spawn(Components &&...components) noexcept
{
    auto child = _commands->spawn(std::forward<Components>(components)...);

    child.insert(Parent{_parent});
    _commands->add_child(_parent, child.id());
    return child;
}
