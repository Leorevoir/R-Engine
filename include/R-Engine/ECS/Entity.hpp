#pragma once

#include <R-Engine/Core/BitMask.hpp>
#include <R-Engine/ECS/Component.hpp>
#include <R-Engine/Types.hpp>

#include <memory>
#include <vector>

namespace r {

namespace ecs {

class Entity : public NonCopyable
{
    public:
        explicit Entity(const u64 id, const bool active = true);

        u64 getID() const;
        bool isActive() const;

        const std::vector<std::unique_ptr<Component>> &getComponents() const;
        const core::BitMask &getEnabledComponents() const;

        void setActive(const bool active);

    private:
        u64 _id;
        bool _active;
        std::vector<std::unique_ptr<Component>> _components;
        core::BitMask _enabled_components;
};

}// namespace ecs

}// namespace r
