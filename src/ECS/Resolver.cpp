#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Resolver.hpp>

/**
* public
*/

r::ecs::Resolver::Resolver(Scene *s, CommandBuffer *cmd) : _scene(s), _cmd_buffer(cmd)
{
    /* __ctor__ */
}

r::ecs::Commands r::ecs::Resolver::resolve(std::type_identity<Commands>)
{
    return Commands(_cmd_buffer);
}
