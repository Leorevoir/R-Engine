#pragma once

#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>

namespace r {

/**
 * @brief Basic theme configuration for UI elements.
 */
struct R_ENGINE_API UiTheme {
    Color panel_bg          = {30, 30, 38, 230};
    Color button_bg         = {56, 58, 66, 255};
    Color button_bg_hover   = {76, 78, 86, 255};
    Color button_bg_pressed = {96, 98, 106, 255};
    Color text              = {230, 230, 230, 255};
    u8    padding           = 6;
};

} /* namespace r */
