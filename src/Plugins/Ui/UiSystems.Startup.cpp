/**
 * \file UiSystems.Startup.cpp
 * \brief Startup and frame update utilities for the UI plugin.
 */
#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <atomic>

namespace r::ui {

void startup_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<UiTheme> theme, r::ecs::Res<UiFonts> fonts) noexcept
{
    (void) theme;
    (void) fonts;
    r::Logger::info(std::string{"UiPlugin startup. DebugOverlay="} + (cfg.ptr->show_debug_overlay ? "on" : "off"));

    r::Application::quit.store(false, std::memory_order_relaxed);
}

void update_system(r::ecs::ResMut<UiEvents> events, r::ecs::ResMut<UiInputState> input, r::ecs::Res<r::UserInput> ui,
    r::ecs::ResMut<UiPluginConfig> cfg) noexcept
{
    events.ptr->pressed.clear();
    events.ptr->released.clear();
    events.ptr->clicked.clear();
    events.ptr->entered.clear();
    events.ptr->left.clear();
    events.ptr->focus_changed.clear();
    events.ptr->blurred.clear();
    events.ptr->value_changed.clear();

    const ::Vector2 mp = GetMousePosition();
    input.ptr->mouse_position = {mp.x, mp.y};
    input.ptr->mouse_left_pressed = ui.ptr->isMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input.ptr->mouse_left_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    input.ptr->mouse_left_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    /* Debug toggle: F1 to draw bounds */
    if (ui.ptr->isKeyPressed(KEY_F1)) {
        cfg.ptr->debug_draw_bounds = !cfg.ptr->debug_draw_bounds;
    }
}

void clear_click_state_system(r::ecs::ResMut<UiInputState> input) noexcept
{
    input.ptr->last_clicked = r::ecs::NULL_ENTITY;
}

}// namespace r::ui
