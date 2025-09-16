#include <R-Engine/ECS/Command.hpp>

/**
* public
*/

r::ecs::Commands::Commands(r::ecs::Scene *scene) noexcept : _scene(scene)
{
    /* __ctor__ */
}

r::ecs::Entity r::ecs::Commands::spawn() const noexcept
{
    return _scene ? _scene->create_entity() : Entity{0};
}
