/**
 * \file UiSystems.ScrollInput.cpp
 * \brief Mouse wheel handling for scrollable UI containers.
 */
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <limits>
#include <unordered_map>

namespace r::ui {

void scroll_input_system(r::ecs::Res<r::UiInputState> state, r::ecs::Res<r::UserInput> /*input*/, r::ecs::ResMut<r::UiEvents> events,
    r::ecs::Query<r::ecs::Optional<r::ecs::Parent>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::UiScroll>,
        r::ecs::Optional<r::ecs::Children>>
        q) noexcept
{
    const float wheel = GetMouseWheelMove();
    if (wheel == 0.0f)
        return;

    std::unordered_map<r::ecs::Entity, r::ecs::Entity> parents;
    std::unordered_map<r::ecs::Entity, r::Style> styles;
    std::unordered_map<r::ecs::Entity, r::UiScroll *> scrolls;
    std::unordered_map<r::ecs::Entity, r::ecs::Entity> parent_from_children;

    for (auto it = q.begin(); it != q.end(); ++it) {
        auto [parent_opt, style_opt, scroll_opt, children_opt] = *it;
        const auto id = static_cast<r::ecs::Entity>(it.entity());
        parents[id] = parent_opt.ptr ? parent_opt.ptr->entity : r::ecs::NULL_ENTITY;
        if (style_opt.ptr)
            styles[id] = *style_opt.ptr;
        if (scroll_opt.ptr)
            scrolls[id] = const_cast<r::UiScroll *>(scroll_opt.ptr);
        if (children_opt.ptr) {
            for (auto child : children_opt.ptr->entities) {
                parent_from_children[child] = id;
            }
        }
    }

    constexpr auto PLACEHOLDER_THRESHOLD = std::numeric_limits<r::ecs::Entity>::max() / 2u;
    for (auto &kv : parents) {
        auto it = parent_from_children.find(kv.first);
        if (it != parent_from_children.end()) {
            kv.second = it->second;
        } else if (kv.second >= PLACEHOLDER_THRESHOLD) {
            kv.second = r::ecs::NULL_ENTITY;
        }
    }

    auto e = state.ptr->hovered;
    const float step = 40.f;
    while (e != r::ecs::NULL_ENTITY) {
        auto sit = scrolls.find(e);
        auto stit = styles.find(e);
        const bool has_clip = (stit != styles.end()) && (stit->second.clip_children || stit->second.overflow_clip);
        if (sit != scrolls.end() && has_clip) {
            sit->second->y += wheel * step;
            events.ptr->value_changed.push_back(e);
            break;
        }
        auto pit = parents.find(e);
        if (pit == parents.end())
            break;
        e = pit->second;
    }
}

}// namespace r::ui
