/**
 * \file UiSystems.ScrollInput.cpp
 * \brief Mouse wheel handling for scrollable UI containers.
 */
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <unordered_map>

namespace r::ui {

void scroll_input_system(
    r::ecs::Res<r::UiInputState> state,
    r::ecs::Res<r::UserInput> /*input*/,
    r::ecs::ResMut<r::UiEvents> events,
    r::ecs::Query<r::ecs::Optional<r::UiId>, r::ecs::Optional<r::UiParent>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::UiScroll>> q) noexcept
{
    (void)events;
    const float wheel = GetMouseWheelMove();
    if (wheel == 0.0f) return;

    std::unordered_map<u32, u32> parents;
    std::unordered_map<u32, r::Style> styles;
    std::unordered_map<u32, r::UiScroll*> scrolls;

    for (auto [id_opt, parent_opt, style_opt, scroll_opt] : q) {
        const u32 h = id_opt.ptr ? id_opt.ptr->value : 0u;
        if (h == 0) continue;
        if (parent_opt.ptr) parents[h] = parent_opt.ptr->handle;
        if (style_opt.ptr) styles[h] = *style_opt.ptr;
        if (scroll_opt.ptr) scrolls[h] = const_cast<r::UiScroll*>(scroll_opt.ptr);
    }

    u32 e = state.ptr->hovered;
    const float step = 40.f;
    while (e != 0) {
        auto sit = scrolls.find(e);
        auto stit = styles.find(e);
        const bool has_clip = (stit != styles.end()) && (stit->second.clip_children || stit->second.overflow_clip);
        if (sit != scrolls.end() && has_clip) {
            sit->second->y += wheel * step;
            events.ptr->value_changed.push_back(e);
            break;
        }
        auto pit = parents.find(e);
        if (pit == parents.end()) break;
        e = pit->second;
    }
}

}
