#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/UI/Components.hpp>

// Demo systems to showcase Phase 1 resources

// Configure UI on startup: set overlay text and bind a toggle key (H)
static void setup_ui_demo(r::ecs::ResMut<r::UiPluginConfig> cfg, r::ecs::ResMut<r::InputMap> input_map)
{
    cfg.ptr->overlay_text = "UI Plugin Ready";
    input_map.ptr->bindAction("ToggleOverlay", r::KEYBOARD, KEY_H);
}

// Toggle overlay when pressing the bound action
static void toggle_overlay_system(r::ecs::Res<r::UserInput> userInput, r::ecs::Res<r::InputMap> input_map,
    r::ecs::ResMut<r::UiPluginConfig> cfg)
{
    if (input_map.ptr->isActionPressed("ToggleOverlay", *userInput.ptr)) {
        cfg.ptr->show_debug_overlay = !cfg.ptr->show_debug_overlay;
    }
}

// Spawn a simple UI tree (root -> panel -> child) with fixed sizes
static void spawn_ui_tree(r::ecs::Commands &cmds, r::ecs::Res<r::WindowPluginConfig> win)
{
    const float ww = static_cast<float>(win.ptr->size.width);
    const float wh = static_cast<float>(win.ptr->size.height);

    auto root = cmds.spawn(
        r::UiNode{},
        r::Style{ .width = ww, .height = wh, .background = {245, 245, 245, 255}, .z_index = 0, .margin = 0, .padding = 16 },
        r::ComputedLayout{},
        r::Visibility::Visible
    );

    const r::ecs::Entity root_id = root.id();

    auto panel = cmds.spawn(
        r::UiNode{},
        r::Parent{ root_id },
        r::Style{ .width = 320.f, .height = 180.f, .background = {60, 60, 80, 255}, .z_index = 1, .margin = 16.f, .padding = 8.f },
        r::ComputedLayout{},
        r::Visibility::Visible
    );

    const r::ecs::Entity panel_id = panel.id();

    cmds.spawn(
        r::UiNode{},
        r::Parent{ panel_id },
        r::Style{ .width = 200.f, .height = 40.f, .background = {180, 80, 80, 255}, .z_index = 2, .margin = 12.f, .padding = 4.f },
        r::ComputedLayout{},
        r::Visibility::Visible
    );
}

int main()
{
    r::Application{}
        .add_plugins(
            r::DefaultPlugins{}
                .set(r::WindowPlugin{r::WindowPluginConfig{
                    .size = {960, 540},
                    .title = "R-Engine UI Demo",
                    .cursor = r::WindowCursorState::Visible,
                }})
        )
        .add_plugins(r::UiPlugin{r::UiPluginConfig{ .show_debug_overlay = true, .overlay_text = "UI Plugin Ready" }})
        // Phase 1 usage examples
        .add_systems(r::Schedule::STARTUP, setup_ui_demo, spawn_ui_tree)
        .add_systems(r::Schedule::UPDATE, toggle_overlay_system)
        .run();
}
