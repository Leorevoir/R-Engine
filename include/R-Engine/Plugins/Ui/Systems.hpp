#pragma once

#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/UI/InputState.hpp>
#include <R-Engine/UI/Theme.hpp>
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/UI/Text.hpp>
#include <R-Engine/UI/Image.hpp>
#include <R-Engine/UI/Button.hpp>
#include <R-Engine/UI/Textures.hpp>
#include <R-Engine/UI/Fonts.hpp>
#include <R-Engine/Core/Backend.hpp>

namespace r::ui {

/* Startup / Update */
void startup_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<UiTheme> theme, r::ecs::Res<UiFonts> fonts) noexcept;
void update_system(r::ecs::ResMut<UiEvents> events, r::ecs::ResMut<UiInputState> input, r::ecs::Res<r::UserInput> ui, r::ecs::ResMut<UiPluginConfig> cfg) noexcept;

/* Hierarchy mapping */
void remap_parents_system(r::ecs::Query<r::ecs::Mut<r::Parent>> q, r::ecs::PlaceholderMap map) noexcept;

/* Pointer + keyboard navigation */
void pointer_system(
    r::ecs::ResMut<UiInputState> state,
    r::ecs::ResMut<UiEvents> events,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>, r::ecs::Optional<r::Parent>, r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::UiScroll>, r::ecs::EntityId> q) noexcept;

void keyboard_nav_system(
    r::ecs::Res<r::UserInput> input,
    r::ecs::ResMut<r::UiInputState> state,
    r::ecs::ResMut<r::UiEvents> events,
    r::ecs::Query<r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::Visibility>, r::ecs::EntityId> q);

/* Layout + scroll */
void compute_layout_system(
    r::ecs::Commands &cmds,
    r::ecs::Query<
        r::ecs::Optional<r::Parent>,
        r::ecs::Optional<r::Style>,
        r::ecs::Optional<r::Visibility>,
        r::ecs::Ref<r::UiNode>,
        r::ecs::EntityId
    > q,
    r::ecs::Res<r::WindowPluginConfig> win,
    r::ecs::Res<r::UiTheme> theme);

void scroll_clamp_system(
    r::ecs::Query<r::ecs::Mut<r::UiScroll>, r::ecs::Ref<r::ComputedLayout>, r::ecs::EntityId> scq,
    r::ecs::Query<r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Parent>, r::ecs::EntityId> allq,
    r::ecs::Res<r::UiTheme> theme);

/* Render */
void render_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<r::Camera3d> cam,
    r::ecs::Res<r::UiInputState> input,
    r::ecs::Res<r::UiTheme> theme,
    r::ecs::ResMut<r::UiTextures> textures,
    r::ecs::ResMut<r::UiFonts> fonts,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>, r::ecs::Optional<r::Parent>, r::ecs::Optional<r::UiText>, r::ecs::Optional<r::UiImage>, r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::UiScroll>, r::ecs::EntityId> q) noexcept;

} // namespace r::ui
