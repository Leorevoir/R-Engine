#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/UI/InputState.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <unordered_set>
#include <algorithm>

/* Forward declarations for Phase 5 example systems */
static void apply_ui_events_to_styles(r::ecs::Res<r::UiEvents> events, r::ecs::Res<r::UiInputState> state,
    r::ecs::Query<r::ecs::Mut<r::Style>, r::ecs::EntityId> q);
static void log_ui_clicks(r::ecs::Res<r::UiEvents> events);

/* Demo systems to showcase Phase 1 resources */

/* Configure UI on startup: set overlay text and bind a toggle key (H) */
static void setup_ui_demo(r::ecs::ResMut<r::UiPluginConfig> cfg, r::ecs::ResMut<r::InputMap> input_map)
{
    cfg.ptr->overlay_text = "UI Plugin Ready";
    input_map.ptr->bindAction("ToggleOverlay", r::KEYBOARD, KEY_H);
}

/* Toggle overlay when pressing the bound action */
static void toggle_overlay_system(r::ecs::Res<r::UserInput> userInput, r::ecs::Res<r::InputMap> input_map,
    r::ecs::ResMut<r::UiPluginConfig> cfg)
{
    if (input_map.ptr->isActionPressed("ToggleOverlay", *userInput.ptr)) {
        cfg.ptr->show_debug_overlay = !cfg.ptr->show_debug_overlay;
    }
}

/* Spawn a UI tree to exercise Phase 4 (z-index, borders, clipping) */
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
        r::Style{ .width = 900.f,
                  .height = 220.f,
                  .background = {60, 60, 80, 255},
                  .z_index = 1,
                  .margin = 24.f,
                  .padding = 16.f,
                  .direction = r::LayoutDirection::Row,
                  .justify = r::JustifyContent::SpaceBetween,
                  .align = r::AlignItems::Center,
                  .clip_children = true,
                  .border_thickness = 3.f,
                  .border_color = {240, 240, 240, 255} },
        r::ComputedLayout{},
        r::Visibility::Visible
    );

    const r::ecs::Entity panel_id = panel.id();

    /* Child 1: large red bar (intentionally wider than panel content to demonstrate clipping) */
    auto child1 = cmds.spawn(
        r::UiNode{},
        r::Parent{ panel_id },
        r::Style{ .width = 700.f, .height = 60.f, .background = {180, 80, 80, 255}, .z_index = 2, .margin = 12.f, .padding = 6.f, .border_thickness = 2.f, .border_color = {220, 180, 180, 255} },
        r::ComputedLayout{},
        r::Visibility::Visible
    );

    const r::ecs::Entity child1_id = child1.id();

    /* Nested child inside Child 1 to demonstrate z-index overlay and border */
    cmds.spawn(
        r::UiNode{},
        r::Parent{ child1_id },
        r::Style{ .width = 300.f, .height = 40.f, .background = {230, 200, 120, 255}, .z_index = 10, .margin = 4.f, .padding = 2.f, .border_thickness = 2.f, .border_color = {40, 40, 40, 255} },
        r::ComputedLayout{},
        r::Visibility::Visible
    );

    /* Child 2: medium green block */
    cmds.spawn(
        r::UiNode{},
        r::Parent{ panel_id },
        r::Style{ .width = 260.f, .height = 90.f, .background = {80, 180, 120, 255}, .z_index = 2, .margin = 12.f, .padding = 4.f, .border_thickness = 2.f, .border_color = {220, 255, 220, 255} },
        r::ComputedLayout{},
        r::Visibility::Visible
    );

    /* Child 3: tall blue block (helps visualize align-items: Center) */
    cmds.spawn(
        r::UiNode{},
        r::Parent{ panel_id },
        r::Style{ .width = 80.f, .height = 160.f, .background = {80, 120, 200, 255}, .z_index = 2, .margin = 12.f, .padding = 4.f, .border_thickness = 2.f, .border_color = {210, 220, 255, 255} },
        r::ComputedLayout{},
        r::Visibility::Visible
    );

    /* Second panel (no clipping) to compare behavior */
    auto panel2 = cmds.spawn(
        r::UiNode{},
        r::Parent{ root_id },
        r::Style{ .width = 900.f,
                  .height = 140.f,
                  .background = {60, 60, 80, 255},
                  .z_index = 0,
                  .margin = 24.f,
                  .padding = 12.f,
                  .direction = r::LayoutDirection::Row,
                  .justify = r::JustifyContent::Start,
                  .align = r::AlignItems::Start,
                  .clip_children = false,
                  .border_thickness = 2.f,
                  .border_color = {180, 180, 180, 255} },
        r::ComputedLayout{},
        r::Visibility::Visible
    );
    const r::ecs::Entity panel2_id = panel2.id();
    /* Oversized child spills outside since clipping is disabled */
    cmds.spawn(
        r::UiNode{},
        r::Parent{ panel2_id },
        r::Style{ .width = 1100.f, .height = 80.f, .background = {160, 100, 160, 255}, .z_index = 1, .margin = 8.f, .padding = 6.f, .border_thickness = 2.f, .border_color = {240, 200, 240, 255} },
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
        /* Phase 1 usage examples */
        .add_systems(r::Schedule::STARTUP, setup_ui_demo, spawn_ui_tree)
        .add_systems(r::Schedule::UPDATE, toggle_overlay_system, apply_ui_events_to_styles, log_ui_clicks)
        .run();
}

/**
 * @brief Apply UI events (hover/press/release/click) to styles to visualize Phase 5.
 */
static void apply_ui_events_to_styles(r::ecs::Res<r::UiEvents> events, r::ecs::Res<r::UiInputState> state,
    r::ecs::Query<r::ecs::Mut<r::Style>, r::ecs::EntityId> q)
{
    std::unordered_set<r::ecs::Entity> pressed(events.ptr->pressed.begin(), events.ptr->pressed.end());
    std::unordered_set<r::ecs::Entity> released(events.ptr->released.begin(), events.ptr->released.end());
    std::unordered_set<r::ecs::Entity> entered(events.ptr->entered.begin(), events.ptr->entered.end());
    std::unordered_set<r::ecs::Entity> left(events.ptr->left.begin(), events.ptr->left.end());
    std::unordered_set<r::ecs::Entity> clicked(events.ptr->clicked.begin(), events.ptr->clicked.end());

    for (auto [style, id] : q) {
        const r::ecs::Entity e = id.value;

        if (entered.count(e)) {
            style.ptr->border_thickness = std::max(style.ptr->border_thickness, 2.f);
            style.ptr->border_color = {240, 240, 240, 255};
        }
        if (pressed.count(e)) {
            style.ptr->border_thickness = std::max(style.ptr->border_thickness, 3.f);
            style.ptr->border_color = {255, 230, 0, 255};
        }
        if (released.count(e)) {
            if (state.ptr->hovered == e) {
                style.ptr->border_thickness = std::max(style.ptr->border_thickness, 2.f);
                style.ptr->border_color = {240, 240, 240, 255};
            } else if (state.ptr->focused != e) {
                style.ptr->border_thickness = 0.f;
            }
        }
        if (left.count(e) && state.ptr->focused != e && state.ptr->active != e) {
            style.ptr->border_thickness = 0.f;
        }
        if (clicked.count(e)) {
            r::Color &c = style.ptr->background;
            auto clamp_inc = [](u8 v, int inc) -> u8 { int nv = static_cast<int>(v) + inc; if (nv < 0) nv = 0; if (nv > 255) nv = 255; return static_cast<u8>(nv); };
            c.r = clamp_inc(c.r, 20);
            c.g = clamp_inc(c.g, 20);
            c.b = clamp_inc(c.b, 20);
        }
    }
}

/**
 * @brief Log clicked entities to demonstrate UiEvents consumption.
 */
static void log_ui_clicks(r::ecs::Res<r::UiEvents> events)
{
    for (auto e : events.ptr->clicked) {
        r::Logger::info("UI clicked entity: " + std::to_string(e));
    }
}
