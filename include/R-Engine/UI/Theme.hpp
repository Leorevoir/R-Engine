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
            Color bg_normal = {56, 58, 66, 255};
            Color bg_hover = {76, 78, 86, 255};
            Color bg_pressed = {96, 98, 106, 255};
            Color bg_disabled = {48, 48, 54, 255};

            Color border_normal = {210, 210, 210, 255};
            Color border_hover = {235, 235, 235, 255};
            Color border_pressed = {255, 230, 0, 255};
            Color border_disabled = {120, 120, 120, 255};

            float border_thickness = 2.f;
            Color text = {230, 230, 230, 255};
    } button;
};

} /* namespace r */
