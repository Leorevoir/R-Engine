#pragma once

#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/ECS/Entity.hpp>

namespace r {

namespace ecs {

class System : public NonCopyable
{
    public:
        constexpr System() = default;
        virtual ~System();

        virtual bool update(const core::FrameTime &frame);

        const core::BitMask &getAcceptedComponents() const;

        bool contains(const Entity &entity) const;
        void link(const std::unique_ptr<Entity> &entity);
        void unlink(const std::unique_ptr<Entity> &entity);

    protected:
        std::vector<Entity *> _entities;
        core::BitMask _accepted_components;

    private:
        static inline u64 _max_id = 0;
};

}// namespace ecs

}// namespace r
