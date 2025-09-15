#include <R-Engine/ECS/Scene.hpp>

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/ECS/System.hpp>
#include <algorithm>

/**
* public
*/

r::Scene::Scene(const u64 entity_count)
{
    _entities.reserve(entity_count);
}

r::Scene::~Scene()
{
    _entities.clear();
    _systems.clear();
}

bool r::Scene::update(const core::FrameTime &frame)
{
    if (!_entities.empty()) {
        _sort();
        _refresh();
    }

    for (u64 i = 0; i < _systems.size(); i++) {
        if (!_active_systems[i]) {
            continue;
        }
        if (!_systems[i]->update(frame)) {
            _active_systems.reset(i);
        }
    }

    return _active_systems.empty();
}

/**
* private
*/
// clang-format off
void r::Scene::_sort()
{
    const auto it = std::partition(_entities.begin(), _entities.end(),
    [](const std::unique_ptr<ecs::Entity> &entity) {
        return entity != nullptr && entity->isActive();
    });

    _active_entities = static_cast<u64>(std::distance(_entities.begin(), it));
}
// clang-format on

void r::Scene::_refresh()
{
    for (u64 i = 0; i < _active_entities; ++i) {
        const auto &entity = _entities[i];

        if (!entity || entity->isActive()) {
            continue;
        }

        for (u64 j = 0; j < _systems.size(); ++j) {
            const auto &system = _systems[j];

            if (!system || !_active_systems[j]) {
                continue;
            }

            const core::BitMask match = entity->getEnabledComponents() & system->getAcceptedComponents();
            const bool contains = system->contains(*entity);
            const bool matched = !match.empty();

            if (!contains && matched) {
                system->link(entity);
            } else if (contains && !matched) {
                system->unlink(entity);
            }
        }
    }
}
