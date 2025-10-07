#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>
#include <string>

namespace r {

/**
 * @brief Text component for UI nodes.
 */
struct R_ENGINE_API UiText {
    std::string content;
    int font_size = 18;
    float wrap_width = 0.f; /* 0 -> no wrap; otherwise wrap at this width in px */
    Color color = {230, 230, 230, 255};
    std::string font_path; /* optional custom font; empty -> default */
};

} /* namespace r */
