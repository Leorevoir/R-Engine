#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>
#include <string>

namespace r {

/**
 * @brief Image component for UI nodes.
 */
struct R_ENGINE_API UiImage {
    std::string path;         /* texture file path */
    Color tint = {255, 255, 255, 255};
    bool keep_aspect = true;  /* fit inside rect while preserving aspect */
};

} /* namespace r */

