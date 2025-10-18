/**
 * \file UiSystems.Input.cpp
 * \brief Pointer and keyboard input systems for UI interactions.
 */
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <vector>

namespace r::ui {

/// @brief UI element for input processing with associated metadata
struct InputItem {
        int z;                          ///< Z-index for depth sorting
        size_t ord;                     ///< Creation order for stable sorting
        r::ecs::Entity handle;          ///< Entity handle
        const r::ComputedLayout *layout;///< Computed layout debugrmation
        r::Style style;                 ///< Visual style properties
        r::ecs::Entity parent;          ///< Parent entity for hierarchy
        bool disabled;                  ///< Whether input is disabled
        const r::UiScroll *scroll;      ///< Scroll component if present
};

/// @brief Cached input data for efficient access during input processing
struct InputData {
        std::unordered_map<r::ecs::Entity, const r::ComputedLayout *> layouts;  ///< Layout data by entity
        std::unordered_map<r::ecs::Entity, r::Style> styles;                    ///< Style data by entity
        std::unordered_map<r::ecs::Entity, r::ecs::Entity> parents;             ///< Parent entity mapping
        std::unordered_map<r::ecs::Entity, const r::UiScroll *> scrolls;        ///< Scroll components by entity
        std::unordered_map<r::ecs::Entity, r::ecs::Entity> parent_from_children;///< Reverse parent lookup
        std::unordered_map<r::ecs::Entity, bool> button_enabled;                ///< Button enabled state by entity
};

/// @brief Threshold value for determining placeholder entities in input system
static constexpr auto INPUT_PLACEHOLDER_THRESHOLD = std::numeric_limits<r::ecs::Entity>::max() / 2u;

/**
 * @brief Collects and sorts UI elements for input processing
 * @tparam QueryType Type of the ECS query
 * @param q ECS query containing UI entities
 * @return Pair of sorted input items and associated input data
 */
template<typename QueryType>
static auto collect_input_items(QueryType &q) -> std::pair<std::vector<InputItem>, InputData>
{
    std::vector<InputItem> items;
    items.reserve(256);
    InputData data;

    size_t ord = 0;
    for (auto it = q.begin(); it != q.end(); ++it) {
        auto [node, layout, style_opt, vis_opt, parent_opt, button_opt, scroll_opt, children_opt] = *it;

        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible))
            continue;
        const auto id = static_cast<r::ecs::Entity>(it.entity());
        const auto pid = parent_opt.ptr ? parent_opt.ptr->entity : r::ecs::NULL_ENTITY;
        r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        const bool disabled = button_opt.ptr && button_opt.ptr->disabled;

        items.push_back({s.z_index, ord++, id, layout.ptr, s, pid, disabled, scroll_opt.ptr});
        data.layouts[id] = layout.ptr;
        data.styles[id] = s;
        data.parents[id] = pid;

        if (scroll_opt.ptr)
            data.scrolls[id] = scroll_opt.ptr;
        if (button_opt.ptr)
            data.button_enabled[id] = !disabled;

        if (children_opt.ptr) {
            for (auto child : children_opt.ptr->entities) {
                data.parent_from_children[child] = id;
            }
        }
    }

    std::stable_sort(items.begin(), items.end(), [](const InputItem &a, const InputItem &b) {
        if (a.z != b.z)
            return a.z < b.z;
        return a.ord < b.ord;
    });

    return std::make_pair(std::move(items), std::move(data));
}

/**
 * @brief Sets up parent-child entity mappings for input processing
 * @param items Vector of input items to update with resolved parents
 * @param data Input data containing entity mappings
 */
static void setup_input_entity_mappings(std::vector<InputItem> &items, InputData &data)
{
    for (auto &kv : data.parents) {
        auto it = data.parent_from_children.find(kv.first);
        if (it != data.parent_from_children.end()) {
            kv.second = it->second;
        } else if (kv.second >= INPUT_PLACEHOLDER_THRESHOLD) {
            kv.second = r::ecs::NULL_ENTITY;
        }
    }

    for (auto &item : items) {
        auto it = data.parents.find(item.handle);
        if (it != data.parents.end()) {
            item.parent = it->second;
        }
        if (item.parent >= INPUT_PLACEHOLDER_THRESHOLD) {
            item.parent = r::ecs::NULL_ENTITY;
        }
    }
}

/**
 * @brief Tests if a point is inside a rectangle
 * @param px X coordinate of the point
 * @param py Y coordinate of the point
 * @param l Layout rectangle to test against
 * @return True if point is inside rectangle, false otherwise
 */
static auto point_in_rectangle(float px, float py, const r::ComputedLayout *l) -> bool
{
    return (px >= l->x && px <= l->x + l->w && py >= l->y && py <= l->y + l->h);
}

static auto calculate_input_scroll_offset(const InputData &data, r::ecs::Entity e) -> std::pair<float, float>
{
    float sx = 0.f, sy = 0.f;
    auto p = e;
    while (p != r::ecs::NULL_ENTITY) {
        auto pit = data.parents.find(p);
        if (pit == data.parents.end())
            break;
        p = pit->second;
        auto sit = data.styles.find(p);
        auto scit = data.scrolls.find(p);
        if (sit != data.styles.end() && (sit->second.clip_children || sit->second.overflow_clip) && scit != data.scrolls.end()
            && scit->second) {
            sx -= scit->second->x;
            sy -= scit->second->y;
        }
    }
    return std::make_pair(sx, sy);
}

static auto is_inside_with_clipping(const InputItem &item, const InputData &data, float mx, float my) -> bool
{
    if (!point_in_rectangle(mx, my, item.layout))
        return false;

    auto p = item.parent;
    while (p != r::ecs::NULL_ENTITY) {
        auto psit = data.styles.find(p);
        auto plit = data.layouts.find(p);
        if (psit != data.styles.end() && plit != data.layouts.end()) {
            const r::Style &ps = psit->second;
            const r::ComputedLayout *pl = plit->second;
            if (ps.clip_children || ps.overflow_clip) {
                float cx = pl->x + ps.padding;
                float cy = pl->y + ps.padding;
                float cw = pl->w - ps.padding * 2.f;
                float ch = pl->h - ps.padding * 2.f;
                if (!(mx >= cx && mx <= cx + cw && my >= cy && my <= cy + ch))
                    return false;
            }
        }
        auto pit = data.parents.find(p);
        if (pit == data.parents.end())
            break;
        p = pit->second;
    }
    return true;
}

static auto resolve_click_target(const InputData &data, r::ecs::Entity e) -> r::ecs::Entity
{
    auto current = e;
    while (current != r::ecs::NULL_ENTITY) {
        if (auto bit = data.button_enabled.find(current); bit != data.button_enabled.end()) {
            return bit->second ? current : r::ecs::NULL_ENTITY;
        }
        auto pit = data.parents.find(current);
        if (pit == data.parents.end())
            break;
        current = pit->second;
    }
    return e;
}

/**
 * @brief Finds the topmost UI entity under the mouse cursor
 * @param items Vector of input items sorted by z-order
 * @param data Input data for clipping and scroll calculations
 * @param mx Mouse X coordinate
 * @param my Mouse Y coordinate
 * @return Entity under cursor or NULL_ENTITY if none
 */
static auto find_hovered_entity(const std::vector<InputItem> &items, const InputData &data, float mx, float my) -> r::ecs::Entity
{
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        if (it->disabled)
            continue;
        auto [sx, sy] = calculate_input_scroll_offset(data, it->handle);
        if (is_inside_with_clipping(*it, data, mx - sx, my - sy)) {
            return resolve_click_target(data, it->handle);
        }
    }
    return r::ecs::NULL_ENTITY;
}

/**
 * @brief Handles mouse hover enter/leave events
 * @param hovered Currently hovered entity
 * @param state UI input state to update
 * @param events UI events to generate for hover changes
 */
static void handle_hover_events(r::ecs::Entity hovered, r::ecs::ResMut<UiInputState> state, r::ecs::ResMut<UiEvents> events)
{
    if (hovered != state.ptr->hovered) {
        if (state.ptr->hovered != r::ecs::NULL_ENTITY) {
            events.ptr->left.push_back(state.ptr->hovered);
        }
        if (hovered != r::ecs::NULL_ENTITY) {
            events.ptr->entered.push_back(hovered);
        }
        state.ptr->prev_hovered = state.ptr->hovered;
        state.ptr->hovered = hovered;
    }
}

/**
 * @brief Handles mouse press, release and click events
 * @param hovered Currently hovered entity
 * @param state UI input state to update
 * @param events UI events to generate for click interactions
 */
static void handle_click_events(r::ecs::Entity hovered, r::ecs::ResMut<UiInputState> state, r::ecs::ResMut<UiEvents> events)
{
    if (state.ptr->mouse_left_pressed && hovered != r::ecs::NULL_ENTITY) {
        if (state.ptr->focused != r::ecs::NULL_ENTITY && state.ptr->focused != hovered) {
            events.ptr->blurred.push_back(state.ptr->focused);
            events.ptr->focus_changed.push_back(hovered);
        }
        state.ptr->active = hovered;
        state.ptr->focused = hovered;
        events.ptr->pressed.push_back(hovered);
    }

    if (state.ptr->mouse_left_released) {
        if (state.ptr->active != r::ecs::NULL_ENTITY) {
            events.ptr->released.push_back(state.ptr->active);
            if (hovered == state.ptr->active) {
                events.ptr->clicked.push_back(state.ptr->active);
                state.ptr->last_clicked = state.ptr->active;
            }
        }
        state.ptr->active = r::ecs::NULL_ENTITY;
    }
}

void pointer_system(r::ecs::ResMut<UiInputState> state, r::ecs::ResMut<UiEvents> events,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>,
        r::ecs::Optional<r::ecs::Parent>, r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::UiScroll>, r::ecs::Optional<r::ecs::Children>>
        q) noexcept
{
    auto [items, data] = collect_input_items(q);
    setup_input_entity_mappings(items, data);

    const float mx = state.ptr->mouse_position.x;
    const float my = state.ptr->mouse_position.y;
    auto hovered = find_hovered_entity(items, data, mx, my);

    handle_hover_events(hovered, state, events);
    handle_click_events(hovered, state, events);
}

void keyboard_nav_system(r::ecs::Res<r::UserInput> input, r::ecs::ResMut<r::UiInputState> state, r::ecs::ResMut<r::UiEvents> events,
    r::ecs::Query<r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::Visibility>> q)
{
    std::vector<u32> order;
    order.reserve(64);
    for (auto it = q.begin(); it != q.end(); ++it) {
        auto [btn_opt, vis_opt] = *it;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible))
            continue;
        const bool focusable = (btn_opt.ptr && !btn_opt.ptr->disabled);
        if (focusable)
            order.push_back(static_cast<u32>(it.entity()));
    }
    if (order.empty())
        return;

    auto find_index = [&](u32 h) {
        for (size_t i = 0; i < order.size(); ++i) {
            if (order[i] == h)
                return static_cast<int>(i);
        }
        return -1;
    };

    const bool shift = input.ptr->isKeyPressed(KEY_LEFT_SHIFT) || input.ptr->isKeyPressed(KEY_RIGHT_SHIFT);
    if (input.ptr->isKeyPressed(KEY_TAB)) {
        const u32 prev = state.ptr->focused;
        int idx = find_index(state.ptr->focused);
        if (idx < 0)
            idx = 0;
        else
            idx = shift ? (idx - 1 + static_cast<int>(order.size())) % static_cast<int>(order.size())
                        : (idx + 1) % static_cast<int>(order.size());
        state.ptr->focused = order[static_cast<size_t>(idx)];
        if (prev != r::ecs::NULL_ENTITY && prev != state.ptr->focused)
            events.ptr->blurred.push_back(prev);
        events.ptr->focus_changed.push_back(state.ptr->focused);
        r::Logger::debug(std::string{"UI focus -> handle "} + std::to_string(state.ptr->focused));
    }

    const bool activate = input.ptr->isKeyPressed(KEY_ENTER) || input.ptr->isKeyPressed(KEY_SPACE);
    if (activate && state.ptr->focused != r::ecs::NULL_ENTITY) {
        const u32 h = state.ptr->focused;
        state.ptr->active = h;
        events.ptr->pressed.push_back(h);
        events.ptr->released.push_back(h);
        events.ptr->clicked.push_back(h);
    }
}

}// namespace r::ui
