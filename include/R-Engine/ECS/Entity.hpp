#pragma once

#include <R-Engine/Types.hpp>

namespace r {

namespace ecs {

using Entity = u32;

static constexpr Entity NULL_ENTITY = static_cast<Entity>(-1);

}// namespace ecs

}// namespace r
