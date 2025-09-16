#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/ECS/Scene.hpp>

namespace r {

namespace ecs {

struct Commands {
    public:
        explicit Commands(Scene *scene = nullptr);

        Entity spawn() const;

        template<typename T>
        void add_component(Entity e, T comp) const;

    private:
        Scene *_scene;
};

}// namespace ecs

}// namespace r

#include "Inline/Command.inl"
