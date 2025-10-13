#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Core/Backend.hpp>

#include <string>
#include <unordered_map>

namespace r {

/**
 * @brief Font cache for UI text. Loads fonts on demand and unloads on destruction.
 */
struct R_ENGINE_API UiFonts {
    std::unordered_map<std::string, ::Font> cache;
    UiFonts() = default;
    ~UiFonts();

    const ::Font *load(const std::string &path);
    void unload(const std::string &path);
};

} /* namespace r */
