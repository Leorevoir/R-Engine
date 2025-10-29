#pragma once

#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>

namespace r {

// clang-format off

enum class PostProcessingState {
    Grayscale       = 0,
    Posterization   = 1,
    DreamVision     = 2,
    Pixelizer       = 3,
    CrossHatching   = 4,
    CrossStitching  = 5,
    PredatorView    = 6,
    Scanlines       = 7,
    Fisheye         = 8,
    Sobel           = 9,
    Bloom           = 10,
    Blur            = 11,
    Contrast        = 12,
    Protanopia      = 13,
    Deuteranopia    = 14,
    Tritanopia      = 15,
    Disabled        = 16,
};

// clang-format on

struct R_ENGINE_API PostProcessingPluginConfig final {
        PostProcessingState state = PostProcessingState::Disabled;
        f32 contrast_level = 1.5f;
};

class R_ENGINE_API PostProcessingPlugin final : public Plugin
{
    public:
        PostProcessingPlugin(PostProcessingPluginConfig config = {}) noexcept;

        void build(Application &app) override;

    private:
        PostProcessingPluginConfig _config;
};

}// namespace r
