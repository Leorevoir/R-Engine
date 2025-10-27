#include <R-Engine/Application.hpp>

#include <R-Engine/Components/Transform3d.hpp>

#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>
#include <R-Engine/Plugins/PostProcessingPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

// clang-format off

struct SpinningCube final {
};

static void setup_scene_system(r::ecs::Commands commands, r::ecs::ResMut<r::Meshes> meshes)
{
    commands.spawn(
        SpinningCube{},
        r::Mesh3d{
            .id = meshes.ptr->add(r::Mesh3d::Cube(2.0f)),
            .color = {255, 100, 50, 255}
        },
        r::Transform3d{.position = {0.0f, 1.0f, 0.0f}}
    );
}

static void spinning_cube_system(
    r::ecs::Query<r::ecs::Mut<r::Transform3d>, r::ecs::With<SpinningCube>> query,
    r::ecs::Res<r::core::FrameTime> time
) noexcept
{
    for (auto [transform, _] : query) {
        transform.ptr->rotation.y += 0.5f * time.ptr->delta_time;
        transform.ptr->rotation.z += 0.3f * time.ptr->delta_time;
    }
}

static void post_processing_switcher_system(
    r::ecs::Res<r::UserInput> input,
    r::ecs::ResMut<r::PostProcessingPluginConfig> post_fx_config
)
{
    const i32 total_states = static_cast<i32>(r::PostProcessingState::Disabled) + 1;
    i32 current_state = static_cast<i32>(post_fx_config.ptr->state);

    if (input.ptr->isKeyPressed(KEY_RIGHT)) {
        ++current_state;
        if (current_state >= total_states) {
            current_state = 0;
        }
    }

    if (input.ptr->isKeyPressed(KEY_LEFT)) {
        --current_state;
        if (current_state < 0) {
            current_state = total_states - 1;
        }
    }

    post_fx_config.ptr->state = static_cast<r::PostProcessingState>(current_state);
}

static void ui_info_system(const r::ecs::Res<r::PostProcessingPluginConfig> config)
{
    const char *effect_name = "Unknown";

    switch (config.ptr->state) {
        case r::PostProcessingState::Grayscale:
            effect_name = "Grayscale";
            break;
        case r::PostProcessingState::Posterization:
            effect_name = "Posterization";
            break;
        case r::PostProcessingState::DreamVision:
            effect_name = "Dream Vision";
            break;
        case r::PostProcessingState::Pixelizer:
            effect_name = "Pixelizer";
            break;
        case r::PostProcessingState::CrossHatching:
            effect_name = "Cross-Hatching";
            break;
        case r::PostProcessingState::CrossStitching:
            effect_name = "Cross-Stitching";
            break;
        case r::PostProcessingState::PredatorView:
            effect_name = "Predator View";
            break;
        case r::PostProcessingState::Scanlines:
            effect_name = "Scanlines";
            break;
        case r::PostProcessingState::Fisheye:
            effect_name = "Fisheye";
            break;
        case r::PostProcessingState::Sobel:
            effect_name = "Sobel";
            break;
        case r::PostProcessingState::Bloom:
            effect_name = "Bloom";
            break;
        case r::PostProcessingState::Blur:
            effect_name = "Blur";
            break;
        case r::PostProcessingState::Disabled:
            effect_name = "Disabled";
            break;
    }

    DrawText("Use LEFT/RIGHT arrow keys to change effect", 10, 10, 20, RAYWHITE);
    const std::string current_effect_text = "Current Effect: " + std::string(effect_name);
    DrawText(current_effect_text.c_str(), 10, 40, 20, LIME);
}

static const auto g_window_plugin_config = r::WindowPluginConfig
{
    .size = {1280, 720},
    .title = "Post-Processing Demo",
};

i32 main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}
            .set(r::WindowPlugin(g_window_plugin_config))
            .set(r::RenderPlugin({.clear_color = {25, 25, 35, 255}}))
        )
        .add_systems<setup_scene_system>(r::Schedule::STARTUP)
        .add_systems<spinning_cube_system, post_processing_switcher_system>(r::Schedule::UPDATE)
        .add_systems<ui_info_system>(r::Schedule::RENDER_2D)
        .run();

    return 0;
}
