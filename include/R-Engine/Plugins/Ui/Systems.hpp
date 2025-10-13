#pragma once

#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/UI/Button.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/UI/Fonts.hpp>
#include <R-Engine/UI/Image.hpp>
#include <R-Engine/UI/InputState.hpp>
#include <R-Engine/UI/Text.hpp>
#include <R-Engine/UI/Textures.hpp>
#include <R-Engine/UI/Theme.hpp>

namespace r::ui {

/**
 * @brief Initialize UI resources (fonts/theme) and log configuration.
 */
void startup_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<UiTheme> theme, r::ecs::Res<UiFonts> fonts) noexcept;

/**
 * @brief Clear transient UiEvents and sample input devices each frame.
 */
void update_system(r::ecs::ResMut<UiEvents> events, r::ecs::ResMut<UiInputState> input, r::ecs::Res<r::UserInput> ui,
    r::ecs::ResMut<UiPluginConfig> cfg) noexcept;

/**
 * @brief Clear click state at the end of each frame after all systems have processed it.
 */
void clear_click_state_system(r::ecs::ResMut<UiInputState> input) noexcept;

/**
 * @brief Pointer hit-test and interactions (hover/pressed/released/clicked/focus).
 */
void pointer_system(r::ecs::ResMut<UiInputState> state, r::ecs::ResMut<UiEvents> events,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>,
        r::ecs::Optional<r::ecs::Parent>, r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::UiScroll>, r::ecs::Optional<r::ecs::Children>>
        q) noexcept;

/**
 * @brief Keyboard navigation (TAB/Shift+TAB focus, Enter/Space activate).
 */
void keyboard_nav_system(r::ecs::Res<r::UserInput> input, r::ecs::ResMut<r::UiInputState> state, r::ecs::ResMut<r::UiEvents> events,
    r::ecs::Query<r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::Visibility>> q);

/**
 * @brief Apply mouse wheel input to the closest scrollable ancestor under the pointer.
 */
void scroll_input_system(r::ecs::Res<r::UiInputState> state, r::ecs::Res<r::UserInput> input, r::ecs::ResMut<r::UiEvents> events,
    r::ecs::Query<r::ecs::Optional<r::ecs::Parent>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::UiScroll>,
        r::ecs::Optional<r::ecs::Children>>
        q) noexcept;

/**
 * @brief Compute recursive flex-like layout for all UiNodes.
 */
void compute_layout_system(r::ecs::Query<r::ecs::Mut<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>,
                               r::ecs::Optional<r::ecs::Parent>, r::ecs::Optional<r::UiScroll>, r::ecs::Optional<r::ecs::Children>>
                               q,
    r::ecs::Res<r::UiTheme> theme);

/**
 * @brief Clamp UiScroll offsets to valid content bounds.
 */
void scroll_clamp_system(r::ecs::Query<r::ecs::Mut<r::UiScroll>, r::ecs::Ref<r::ComputedLayout>> scq,
    r::ecs::Query<r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::ecs::Parent>,
        r::ecs::Optional<r::ecs::Children>>
        allq,
    r::ecs::Res<r::UiTheme> theme);

/**
 * @brief Issue draw calls (background, borders, images, text, debug overlay, scrollbars).
 */
void render_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<r::Camera3d> cam, r::ecs::Res<r::UiInputState> input,
    r::ecs::Res<r::UiTheme> theme, r::ecs::ResMut<r::UiTextures> textures, r::ecs::ResMut<r::UiFonts> fonts,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>,
        r::ecs::Optional<r::ecs::Parent>, r::ecs::Optional<r::UiText>, r::ecs::Optional<r::UiImage>, r::ecs::Optional<r::UiButton>,
        r::ecs::Optional<r::UiScroll>, r::ecs::Optional<r::ecs::Children>>
        q) noexcept;

}// namespace r::ui
