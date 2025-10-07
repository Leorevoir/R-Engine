#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <R-Engine/Core/Logger.hpp>

namespace r::ui {

void startup_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<UiTheme> theme, r::ecs::Res<UiFonts> fonts) noexcept
{
    (void)theme;
    (void)fonts;
    r::Logger::info(std::string{"UiPlugin startup. DebugOverlay="} + (cfg.ptr->show_debug_overlay ? "on" : "off"));
}

void update_system(r::ecs::ResMut<UiEvents> events, r::ecs::ResMut<UiInputState> input, r::ecs::Res<r::UserInput> ui, r::ecs::ResMut<UiPluginConfig> cfg) noexcept
{
    events.ptr->pressed.clear();
    events.ptr->released.clear();
    events.ptr->clicked.clear();
    events.ptr->entered.clear();
    events.ptr->left.clear();

    const ::Vector2 mp = GetMousePosition();
    input.ptr->mouse_position = {mp.x, mp.y};
    input.ptr->mouse_left_pressed = ui.ptr->isMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input.ptr->mouse_left_released = ui.ptr->isMouseButtonReleased(MOUSE_BUTTON_LEFT);
    input.ptr->mouse_left_down = ui.ptr->isMouseButtonDown(MOUSE_BUTTON_LEFT);

    /* Debug toggle: F1 to draw bounds */
    if (ui.ptr->isKeyPressed(KEY_F1)) {
        cfg.ptr->debug_draw_bounds = !cfg.ptr->debug_draw_bounds;
    }
}

void remap_parents_system(
    r::ecs::Query<r::ecs::Mut<r::Parent>> q,
    r::ecs::PlaceholderMap map) noexcept
{
    if (!map.ptr) return;
    for (auto [parent] : q) {
        auto it = map.ptr->find(parent.ptr->id);
        if (it != map.ptr->end()) {
            parent.ptr->id = it->second;
        }
    }
}

} // namespace r::ui

