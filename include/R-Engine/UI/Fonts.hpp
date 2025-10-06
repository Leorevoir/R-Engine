#pragma once

#include <R-Engine/R-EngineExport.hpp>

namespace r {

/**
 * @brief Placeholder UI fonts cache (skeleton).
 * @details Phase 1: keep structure minimal, to be extended later
 *          with font handles and lifetime management.
 */
struct R_ENGINE_API UiFonts {
    bool initialized = false; /* placeholder, avoids empty struct warnings */
};

} /* namespace r */
