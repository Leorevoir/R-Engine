#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Resolver.hpp>

/**
* public
*/

r::ecs::Resolver::Resolver(Scene *s) : _scene(s)
{
    /* __ctor__ */
}

r::ecs::Commands r::ecs::Resolver::resolve(std::type_identity<Commands>)
{
    return Commands(_scene);
}
