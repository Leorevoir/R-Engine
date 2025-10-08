#include <R-Engine/ECS/Command.hpp>

/**
* public
*/

/**
 * EntityCommands
 */

r::ecs::EntityCommands::EntityCommands(r::ecs::CommandBuffer *buffer, Entity entity) noexcept : _buffer(buffer), _entity(entity)
{
    /* __ctor__ */
}

r::ecs::Entity r::ecs::EntityCommands::id() const noexcept
{
    return _entity;
}

/**
 * CommandBuffer
 */

void r::ecs::CommandBuffer::apply(Scene &scene)
{
    scene.clear_command_buffer_placeholder_map();
    for (const auto &command : _commands) {
        command(scene);
    }
    _commands.clear();
}

void r::ecs::CommandBuffer::despawn(Entity e)
{
    _add_command([e](Scene &scene) {
        const auto &map = scene.get_command_buffer_placeholder_map();
        const auto it = map.find(e);
        const Entity real_entity = (it != map.end()) ? it->second : e;

        scene.destroy_entity(real_entity);
    });
}

r::ecs::Entity r::ecs::CommandBuffer::spawn_entity()
{
    const Entity placeholder = _next_placeholder--;

    _add_command([placeholder](Scene &scene) {
        const Entity real_entity = scene.create_entity();

        scene.map_command_buffer_placeholder(placeholder, real_entity);
    });

    return placeholder;
}

void r::ecs::CommandBuffer::add_child(Entity parent, Entity child)
{
    _add_command([parent, child](Scene &scene) {
        const auto &map = scene.get_command_buffer_placeholder_map();
        const auto parent_it = map.find(parent);
        const auto child_it = map.find(child);
        const Entity real_parent = (parent_it != map.end()) ? parent_it->second : parent;
        const Entity real_child = (child_it != map.end()) ? child_it->second : child;

        if (scene.has_component<Children>(real_parent)) {
            auto children = scene.get_component_ptr<Children>(real_parent);

            children->entities.push_back(real_child);
        } else {
            scene.add_component(real_parent, Children{{real_child}});
        }
    });
}

r::ecs::Commands *r::ecs::CommandBuffer::get_commands() noexcept
{
    if (!_commands_wrapper) {
        _commands_wrapper = new Commands(this);
    }
    return _commands_wrapper;
}

void r::ecs::CommandBuffer::_add_command(std::function<void(Scene &)> &&command)
{
    _commands.emplace_back(std::move(command));
}

/**
* Commands
*/

r::ecs::Commands::Commands(CommandBuffer *buffer) noexcept : _buffer(buffer)
{
    /* __ctor__ */
}

r::ecs::EntityCommands r::ecs::Commands::spawn() noexcept
{
    const Entity placeholder = _buffer ? _buffer->spawn_entity() : 0;

    return EntityCommands(_buffer, placeholder);
}

r::ecs::EntityCommands r::ecs::Commands::entity(Entity e) noexcept
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

void r::ecs::Commands::despawn(Entity e) noexcept
{
    if (_buffer) {
        _buffer->despawn(e);
    }
}

void r::ecs::Commands::add_child(Entity parent, Entity child) noexcept
{
    if (_buffer) {
        _buffer->add_child(parent, child);
    }
}
