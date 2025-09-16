#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/ECS/Scene.hpp>

namespace r {

namespace ecs {

struct Commands {
    public:
        explicit Commands(Scene *scene = nullptr) : _scene(scene)
        {
            /* __ctor__ */
        }

        Entity spawn() const
        {
            return _scene ? _scene->create_entity() : 0;
        }

        template<typename T>
        void add_component(Entity e, T comp) const
        {
            if (_scene) {
                _scene->add_component<T>(e, std::move(comp));
            }
        }

    private:
        Scene *_scene;
};

}// namespace ecs

}// namespace r
