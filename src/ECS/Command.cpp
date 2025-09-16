#include <R-Engine/ECS/Command.hpp>

/**
* public
*/

r::ecs::Commands::Commands(r::ecs::Scene *scene) : _scene(scene)
{
    /* __ctor__ */
}

r::ecs::Entity r::ecs::Commands::spawn() const
{
    return _scene ? _scene->create_entity() : Entity{0};
}
