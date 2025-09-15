#pragma once

#include <R-Engine/Core/BitMask.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Types.hpp>

#include <memory>

namespace r {

namespace ecs {

class System;
class Entity;

}// namespace ecs

class Scene : public NonCopyable
{
    public:
        constexpr Scene() = default;
        Scene(const u64 entity_count);

        ~Scene();

        bool update(const core::FrameTime &frame);

    private:
        std::vector<std::unique_ptr<ecs::System>> _systems;
        std::vector<std::unique_ptr<ecs::Entity>> _entities;
        core::BitMask _active_systems;

        u64 _active_entities = 0;
        u64 _max_entities = 0;

        void _sort();
        void _refresh();
};

}// namespace r
