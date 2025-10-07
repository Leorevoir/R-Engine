#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>
#include <string>

namespace r {

/**
 * @brief Basic theme configuration for UI elements.
 */
struct R_ENGINE_API UiTheme {
    Color panel_bg = {30, 30, 38, 230};
    Color text = {230, 230, 230, 255};
    u8 padding = 6;
    float spacing = 8.f;            /* default margin when Style.margin == 0 */
    float corner_radius = 0.f;      /* reserved for rounded rects */
    std::string default_font_path;  /* optional default font */
    int default_font_size = 18;

    struct ButtonPalette {
            // A slightly more colorful default palette
            Color bg_normal   = {52, 73, 94, 255};   // blue-gray
            Color bg_hover    = {41, 128, 185, 255}; // blue
            Color bg_pressed  = {33, 97, 140, 255};  // dark blue
            Color bg_disabled = {80, 80, 80, 255};

            Color border_normal   = {200, 205, 215, 255};
            Color border_hover    = {150, 190, 230, 255};
            Color border_pressed  = {255, 196, 0, 255};
            Color border_disabled = {120, 120, 120, 255};

            float border_thickness = 2.f;
            Color text = {245, 245, 245, 255};
    } button;
};

} /* namespace r */
