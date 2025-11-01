#pragma once

#include <R-Engine/Core/Logger.hpp>

template<typename T>
T r::Material3d::get_uniform(const std::string &name) const
{
    const auto it = _uniforms.find(name);

    if (it == _uniforms.end()) {
        Logger::warn("Uniform '" + name + "' not found in material.");
        return T{};
    }

    try {
        return std::any_cast<T>(it->second);
    } catch (const std::bad_any_cast &e) {
        Logger::error("Failed to cast uniform '" + name + "'. Incorrect type requested.");
        return T{};
    }
}
