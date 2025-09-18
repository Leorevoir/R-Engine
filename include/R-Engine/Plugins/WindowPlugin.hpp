#pragma once

#include <R-Engine/Core/Flagable.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/R-EngineExport.hpp>
#include <R-Engine/Types.hpp>

#include <string>

namespace r {

// clang-format off

enum class WindowPluginSettings : u16 {
    FOCUSED        = 1 << 0,
    RESIZABLE      = 1 << 1,
    VISIBLE        = 1 << 2,
    DECORATED      = 1 << 3,
    AUTO_MINIMIZE  = 1 << 4,
    ALWAYS_ON_TOP  = 1 << 5,
    MAXIMIZED      = 1 << 6,
    CENTER_CURSOR  = 1 << 7,
    TRANSPARENT_FB = 1 << 8,
    AUTOFOCUS      = 1 << 9,

    DEFAULT        = FOCUSED | RESIZABLE | VISIBLE | DECORATED,
    BORDERLESS     = FOCUSED | VISIBLE
};
R_ENUM_FLAGABLE(WindowPluginSettings)

struct R_ENGINE_API WindowPluginConfig {
    Vec2u size = {1280, 720};
    std::string title = "R-Engine";
    WindowPluginSettings settings = WindowPluginSettings::DEFAULT;
    u32 frame_per_second = 60;
    u8 antialiasing_samples_count = 1;
};

// clang-format on

class R_ENGINE_API WindowPlugin final : public Plugin
{
    public:
        explicit WindowPlugin(const WindowPluginConfig &config = WindowPluginConfig());

        void build(Application &app) override;

    private:
        WindowPluginConfig _config;
};

}// namespace r
