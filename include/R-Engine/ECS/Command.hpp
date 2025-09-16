#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/ECS/Scene.hpp>

namespace r {

namespace ecs {

struct Commands {
    public:
        explicit Commands(Scene *scene = nullptr) noexcept;

        Entity spawn() const noexcept;

        template<typename T>
        void add_component(Entity e, T comp) const noexcept;

    private:
        Scene *_scene;
};

}// namespace ecs

}// namespace r

#include "Inline/Command.inl"
