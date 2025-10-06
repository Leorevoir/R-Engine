#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>
#include <R-Engine/ECS/Entity.hpp>
#include <vector>

namespace r {

/**
 * @brief Transient UI events emitted each frame (skeleton for now).
 */
struct R_ENGINE_API UiEvents {
    std::vector<ecs::Entity> pressed;
    std::vector<ecs::Entity> released;
    std::vector<ecs::Entity> clicked;
    std::vector<ecs::Entity> entered;
    std::vector<ecs::Entity> left;
};

} /* namespace r */
