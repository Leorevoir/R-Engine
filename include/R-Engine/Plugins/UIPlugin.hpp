#pragma once

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Types.hpp>

#include <string>

namespace r {

struct R_ENGINE_API UiText {
    std::string value;
    u32 size = 16;
};

struct R_ENGINE_API UiRectSize {
    Vec2f size = {0.f, 0.f};
};

struct R_ENGINE_API UiPosition {
    Vec2f pos = {0.f, 0.f};
};

struct R_ENGINE_API UiTextColor {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

struct R_ENGINE_API UiColor {
    unsigned char r = 255, g = 255, b = 255, a = 255;
};

struct R_ENGINE_API UiBorderColor {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

struct R_ENGINE_API UiBorderThickness {
    f32 value = 0.f;
};

struct R_ENGINE_API UiBorderRadius {
    f32 value = 0.f;
};

struct R_ENGINE_API UiZIndex {
    i32 value = 0;
};

struct R_ENGINE_API UiOriginalColor {
    unsigned char r = 0, g = 0, b = 0, a = 255;
};

struct R_ENGINE_API UiOnClickQuit {
};

struct R_ENGINE_API UiTheme {
    f32 hover_dark_percent = 0.5f;
    f32 hover_speed = 14.f;
    f32 restore_speed = 16.f;
    f32 flash_percent = 1.0f;
    f32 flash_speed = 28.f;
    f32 quit_delay = 0.25f;
};

struct R_ENGINE_API UiButton {
};

struct R_ENGINE_API UiButtonState {
    bool hovered = false;
    bool pressed = false;
};

class R_ENGINE_API UiPlugin final : public Plugin
{
    public:
        void build(Application &app) override;
};

} // namespace r
