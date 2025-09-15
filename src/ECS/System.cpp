#include <R-Engine/ECS/System.hpp>

/**
* public
*/

r::ecs::System::~System()
{
    _entities.clear();
}

const r::core::BitMask &r::ecs::System::getAcceptedComponents() const
{
    return _accepted_components;
}

bool r::ecs::System::contains(const r::ecs::Entity &entity) const
{
    const u64 id = entity.getID();

    for (const auto &e : _entities) {
        if (e->getID() == id) {
            return true;
        }
    }
    return false;
}

bool r::ecs::System::update(const r::core::FrameTime __attribute__((unused)) & frame)
{
    return true;
}

void r::ecs::System::link(const std::unique_ptr<r::ecs::Entity> &entity)
{
    if (!contains(*entity)) {
        _entities.push_back(entity.get());
    }
}

void r::ecs::System::unlink(const std::unique_ptr<r::ecs::Entity> &entity)
{
    const u64 id = entity->getID();

    for (auto it = _entities.begin(); it != _entities.end(); ++it) {
        if ((*it)->getID() == id) {
            _entities.erase(it);
            return;
        }
    }
}

/**
* private
*/
