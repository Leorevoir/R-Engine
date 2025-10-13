#pragma once

#include <R-Engine/ECS/Entity.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>
#include <vector>

namespace r {

/**
 * @brief Transient UI events emitted each frame (skeleton for now).
 */
struct R_ENGINE_API UiEvents {
        std::vector<u32> pressed;
        std::vector<u32> released;
        std::vector<u32> clicked;
        std::vector<u32> entered;
        std::vector<u32> left;
        std::vector<u32> focus_changed; /* handle that just gained focus */
        std::vector<u32> blurred;       /* handle that just lost focus */
        std::vector<u32> value_changed; /* generic value-changed: e.g., scroll position */
};

} /* namespace r */
