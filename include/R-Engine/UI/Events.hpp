#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>
#include <vector>

namespace r {

/**
 * @brief Transient UI events emitted each frame (skeleton for now).
 */
struct R_ENGINE_API UiEvents {
    std::vector<u64> _reserved; // keeps structure non-empty
};

} // namespace r

