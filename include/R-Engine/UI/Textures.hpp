#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <string>
#include <unordered_map>

namespace r {

/**
 * @brief Simple texture cache for UI images.
 */
struct R_ENGINE_API UiTextures {
    std::unordered_map<std::string, ::Texture2D> cache;
};

} /* namespace r */
