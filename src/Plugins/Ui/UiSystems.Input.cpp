#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <R-Engine/Core/Logger.hpp>

namespace r::ui {

void pointer_system(
    r::ecs::ResMut<UiInputState> state,
    r::ecs::ResMut<UiEvents> events,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>, r::ecs::Optional<r::UiParent>, r::ecs::Optional<r::UiId>, r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::UiScroll>> q) noexcept
{
    struct Item { int z; size_t ord; u32 handle; const r::ComputedLayout* layout; r::Style style; u32 parent; bool disabled; const r::UiScroll* scroll; };
    std::vector<Item> items; items.reserve(256);
    std::unordered_map<u32, const r::ComputedLayout*> layouts;
    std::unordered_map<u32, r::Style> styles;
    std::unordered_map<u32, u32> parents;
    std::unordered_map<u32, const r::UiScroll*> scrolls;

    size_t ord = 0;
    for (auto [node, layout, style_opt, vis_opt, parent_opt, id_opt, button_opt, scroll_opt] : q) {
        (void)node;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible)) continue;
        const u32 h = id_opt.ptr ? id_opt.ptr->value : 0u;
        const u32 ph = parent_opt.ptr ? parent_opt.ptr->handle : 0u;
        r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        const bool disabled = button_opt.ptr && button_opt.ptr->disabled;
        items.push_back({ s.z_index, ord++, h, layout.ptr, s, ph, disabled, scroll_opt.ptr });
        layouts[h] = layout.ptr; styles[h] = s; parents[h] = ph; if (scroll_opt.ptr) scrolls[h] = scroll_opt.ptr;
    }
    std::stable_sort(items.begin(), items.end(), [](const Item&a,const Item&b){ if (a.z!=b.z) return a.z < b.z; return a.ord < b.ord; });

    auto point_in = [](float px,float py,const r::ComputedLayout* l){ return (px >= l->x && px <= l->x + l->w && py >= l->y && py <= l->y + l->h); };
    auto scroll_of = [&](u32 e){ float sx=0.f, sy=0.f; u32 p = e; while (p!=0){ auto pit=parents.find(p); if (pit==parents.end()) break; p = pit->second; auto sit=styles.find(p); auto scit=scrolls.find(p); if (sit!=styles.end() && (sit->second.clip_children||sit->second.overflow_clip) && scit!=scrolls.end() && scit->second){ sx -= scit->second->x; sy -= scit->second->y; } } return std::pair<float,float>{sx,sy}; };

    auto inside_with_clip = [&](const Item &it, float mx, float my){
        if (!point_in(mx,my,it.layout)) return false;
        u32 p = it.parent;
        while (p != 0) {
            auto psit = styles.find(p);
            auto plit = layouts.find(p);
            if (psit != styles.end() && plit != layouts.end()) {
                const r::Style &ps = psit->second; const r::ComputedLayout *pl = plit->second;
                if (ps.clip_children) {
                    float cx = pl->x + ps.padding, cy = pl->y + ps.padding;
                    float cw = pl->w - ps.padding * 2.f, ch = pl->h - ps.padding * 2.f;
                    if (!(mx >= cx && mx <= cx + cw && my >= cy && my <= cy + ch)) return false;
                }
            }
            auto pit = parents.find(p); if (pit == parents.end()) break; p = pit->second;
        }
        return true;
    };

    const float mx = state.ptr->mouse_position.x; const float my = state.ptr->mouse_position.y;
    u32 hovered = 0;
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        if (it->disabled) continue;
        auto [sx, sy] = scroll_of(it->handle);
        if (inside_with_clip(*it, mx - sx, my - sy)) { hovered = it->handle; break; }
    }

    if (hovered != state.ptr->hovered) {
        if (state.ptr->hovered != 0) events.ptr->left.push_back(state.ptr->hovered);
        if (hovered != 0) events.ptr->entered.push_back(hovered);
        state.ptr->prev_hovered = state.ptr->hovered; state.ptr->hovered = hovered;
    }

    if (state.ptr->mouse_left_pressed && hovered != 0) {
        if (state.ptr->focused != 0 && state.ptr->focused != hovered) {
            events.ptr->blurred.push_back(state.ptr->focused);
            events.ptr->focus_changed.push_back(hovered);
        }
        state.ptr->active = hovered; state.ptr->focused = hovered; events.ptr->pressed.push_back(hovered);
    }
    if (state.ptr->mouse_left_released) {
        if (state.ptr->active != 0) {
            events.ptr->released.push_back(state.ptr->active);
            if (hovered == state.ptr->active) events.ptr->clicked.push_back(state.ptr->active);
        }
        state.ptr->active = 0;
    }
}

void keyboard_nav_system(
    r::ecs::Res<r::UserInput> input,
    r::ecs::ResMut<r::UiInputState> state,
    r::ecs::ResMut<r::UiEvents> events,
    r::ecs::Query<r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::Visibility>, r::ecs::Optional<r::UiId>> q)
{
    std::vector<u32> order; order.reserve(64);
    for (auto [btn_opt, vis_opt, id_opt] : q) {
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible)) continue;
        const bool focusable = (btn_opt.ptr && !btn_opt.ptr->disabled);
        if (focusable) order.push_back(id_opt.ptr ? id_opt.ptr->value : 0u);
    }
    if (order.empty()) return;

    auto find_index = [&](u32 h) {
        for (size_t i = 0; i < order.size(); ++i) {
            if (order[i] == h) return (int)i;
        }
        return -1;
    };

    const bool shift = input.ptr->isKeyPressed(KEY_LEFT_SHIFT) || input.ptr->isKeyPressed(KEY_RIGHT_SHIFT);
    if (input.ptr->isKeyPressed(KEY_TAB)) {
        const u32 prev = state.ptr->focused;
        int idx = find_index(state.ptr->focused);
        if (idx < 0) idx = 0; else idx = shift ? (idx - 1 + (int)order.size()) % (int)order.size() : (idx + 1) % (int)order.size();
        state.ptr->focused = order[(size_t)idx];
        if (prev != 0 && prev != state.ptr->focused) events.ptr->blurred.push_back(prev);
        events.ptr->focus_changed.push_back(state.ptr->focused);
        r::Logger::info(std::string{"UI focus -> handle "} + std::to_string(state.ptr->focused));
    }

    const bool activate = input.ptr->isKeyPressed(KEY_ENTER) || input.ptr->isKeyPressed(KEY_SPACE);
    if (activate && state.ptr->focused != 0) {
        const u32 h = state.ptr->focused;
        state.ptr->active = h;
        events.ptr->pressed.push_back(h);
        events.ptr->released.push_back(h);
        events.ptr->clicked.push_back(h);
    }
}

} // namespace r::ui
