#pragma once

#include <R-Engine/Types.hpp>

namespace r {

namespace ecs {

class Component : public NonCopyable
{
    public:
        constexpr Component() = default;
        virtual ~Component() = default;

    private:
};

}// namespace ecs

}// namespace r
