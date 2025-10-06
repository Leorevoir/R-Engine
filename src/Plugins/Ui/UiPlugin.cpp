#include <R-Engine/Plugins/UiPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/UI/Fonts.hpp>
#include <R-Engine/UI/InputState.hpp>
#include <R-Engine/UI/Theme.hpp>
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace r {

/**
 * INTERNAL SYSTEMS (skeleton)
 */

static void ui_startup_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<UiTheme> theme, r::ecs::Res<UiFonts> fonts) noexcept
{
    (void)theme;
    (void)fonts;
    Logger::info(std::string{"UiPlugin startup. DebugOverlay="} + (cfg.ptr->show_debug_overlay ? "on" : "off"));
}

static void ui_update_system(r::ecs::ResMut<UiEvents> events, r::ecs::ResMut<UiInputState> input, r::ecs::Res<r::UserInput> ui) noexcept
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
}

static void ui_pointer_system(
    r::ecs::ResMut<UiInputState> state,
    r::ecs::ResMut<UiEvents> events,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>, r::ecs::Optional<r::Parent>, r::ecs::EntityId> q) noexcept
{
    struct Item { int z; size_t ord; r::ecs::Entity id; r::ComputedLayout const* layout; r::Style style; r::ecs::Entity parent; };
    std::vector<Item> items; items.reserve(128);
    std::unordered_map<r::ecs::Entity, const r::ComputedLayout*> layouts;
    std::unordered_map<r::ecs::Entity, r::Style> styles;
    std::unordered_map<r::ecs::Entity, r::ecs::Entity> parents;

    size_t ord = 0;
    for (auto [node, layout, style_opt, vis_opt, parent_opt, id] : q) {
        (void)node;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible)) continue;
        r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        items.push_back({ s.z_index, ord++, id.value, layout.ptr, s, parent_opt.ptr ? parent_opt.ptr->id : 0 });
        layouts[id.value] = layout.ptr;
        styles[id.value] = s;
        parents[id.value] = parent_opt.ptr ? parent_opt.ptr->id : 0;
    }
    std::stable_sort(items.begin(), items.end(), [](const Item&a,const Item&b){ if (a.z!=b.z) return a.z < b.z; return a.ord < b.ord; });

    auto point_in = [](float px,float py,const r::ComputedLayout* l){ return (px >= l->x && px <= l->x + l->w && py >= l->y && py <= l->y + l->h); };

    auto inside_with_clip = [&](const Item &it, float mx, float my){
        if (!point_in(mx,my,it.layout)) return false;
        r::ecs::Entity p = it.parent;
        while (p != 0) {
            auto psit = styles.find(p);
            auto plit = layouts.find(p);
            if (psit != styles.end() && plit != layouts.end()) {
                const r::Style &ps = psit->second;
                const r::ComputedLayout *pl = plit->second;
                if (ps.clip_children) {
                    float cx = pl->x + ps.padding;
                    float cy = pl->y + ps.padding;
                    float cw = pl->w - ps.padding * 2.f;
                    float ch = pl->h - ps.padding * 2.f;
                    if (!(mx >= cx && mx <= cx + cw && my >= cy && my <= cy + ch)) return false;
                }
            }
            auto pit = parents.find(p);
            if (pit == parents.end()) break;
            p = pit->second;
        }
        return true;
    };

    const float mx = state.ptr->mouse_position.x;
    const float my = state.ptr->mouse_position.y;

    r::ecs::Entity hovered = 0;
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        if (inside_with_clip(*it, mx, my)) { hovered = it->id; break; }
    }

    if (hovered != state.ptr->hovered) {
        if (state.ptr->hovered != 0) events.ptr->left.push_back(state.ptr->hovered);
        if (hovered != 0) events.ptr->entered.push_back(hovered);
        state.ptr->prev_hovered = state.ptr->hovered;
        state.ptr->hovered = hovered;
    }

    if (state.ptr->mouse_left_pressed && hovered != 0) {
        state.ptr->active = hovered;
        state.ptr->focused = hovered;
        events.ptr->pressed.push_back(hovered);
    }

    if (state.ptr->mouse_left_released) {
        if (state.ptr->active != 0) {
            events.ptr->released.push_back(state.ptr->active);
            if (hovered == state.ptr->active) {
                events.ptr->clicked.push_back(state.ptr->active);
            }
        }
        state.ptr->active = 0;
    }
}

static void ui_remap_parents_system(
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

static constexpr inline ::Camera _to_raylib(const r::Camera3d &c) noexcept
{
    return {
        .position = {c.position.x, c.position.y, c.position.z},
        .target = {c.target.x, c.target.y, c.target.z},
        .up = {c.up.x, c.up.y, c.up.z},
        .fovy = c.fovy,
        .projection = static_cast<int>(c.projection),
    };
}

struct Rect { float x; float y; float w; float h; };

static void _layout_recursive(r::ecs::Commands &cmds,
    const std::unordered_map<r::ecs::Entity, std::vector<r::ecs::Entity>> &children,
    const std::unordered_map<r::ecs::Entity, r::Style> &styles,
    r::ecs::Entity node,
    const Rect &content)
{
    auto it = children.find(node);
    if (it == children.end()) {
        return;
    }
    const r::Style parent_style = [&]() {
        auto ps = styles.find(node);
        return (ps != styles.end()) ? ps->second : r::Style{};
    }();

    const auto &kids = it->second;
    const bool row = parent_style.direction == r::LayoutDirection::Row;
    const float main_size = row ? content.w : content.h;

    float total_main = 0.f;
    for (auto child : kids) {
        const r::Style cs = [&]() {
            auto f = styles.find(child);
            return (f != styles.end()) ? f->second : r::Style{};
        }();
        const float mw = (cs.width > 0.f) ? cs.width : 0.f;
        const float mh = (cs.height > 0.f) ? cs.height : 0.f;
        total_main += (row ? mw : mh) + cs.margin * 2.f;
    }

    float free_space = main_size - total_main;
    if (free_space < 0.f) free_space = 0.f;

    float spacing = 0.f;
    float lead = 0.f;
    switch (parent_style.justify) {
        case r::JustifyContent::Start: lead = 0.f; spacing = 0.f; break;
        case r::JustifyContent::Center: lead = free_space * 0.5f; spacing = 0.f; break;
        case r::JustifyContent::End: lead = free_space; spacing = 0.f; break;
        case r::JustifyContent::SpaceBetween:
            spacing = (kids.size() > 1) ? (free_space / static_cast<float>(kids.size() - 1)) : 0.f;
            lead = 0.f;
            break;
        default: lead = 0.f; spacing = 0.f; break;
    }

    float cursor_x = content.x + lead;
    float cursor_y = content.y + lead;

    for (auto child : kids) {
        const r::Style cs = [&]() {
            auto f = styles.find(child);
            return (f != styles.end()) ? f->second : r::Style{};
        }();

        float cw = (cs.width > 0.f) ? cs.width : (row ? 0.f : content.w - cs.margin * 2.f);
        float ch = (cs.height > 0.f) ? cs.height : (row ? content.h - cs.margin * 2.f : 0.f);

        if (parent_style.align == r::AlignItems::Stretch) {
            if (row && cs.height <= 0.f) ch = content.h - cs.margin * 2.f;
            if (!row && cs.width <= 0.f) cw = content.w - cs.margin * 2.f;
        }

        float x = row ? (cursor_x + cs.margin) : (content.x + cs.margin);
        float y = row ? (content.y + cs.margin) : (cursor_y + cs.margin);

        if (row) {
            switch (parent_style.align) {
                case r::AlignItems::Start: y = content.y + cs.margin; break;
                case r::AlignItems::Center: y = content.y + (content.h - ch) * 0.5f; break;
                case r::AlignItems::End: y = content.y + content.h - ch - cs.margin; break;
                case r::AlignItems::Stretch: y = content.y + cs.margin; break;
                default: y = content.y + cs.margin; break;
            }
        } else {
            switch (parent_style.align) {
                case r::AlignItems::Start: x = content.x + cs.margin; break;
                case r::AlignItems::Center: x = content.x + (content.w - cw) * 0.5f; break;
                case r::AlignItems::End: x = content.x + content.w - cw - cs.margin; break;
                case r::AlignItems::Stretch: x = content.x + cs.margin; break;
                default: x = content.x + cs.margin; break;
            }
        }

        cmds.add_component(child, r::ComputedLayout{ x, y, cw, ch, cs.z_index });

        const Rect child_content{ x + cs.padding, y + cs.padding, cw - cs.padding * 2.f, ch - cs.padding * 2.f };
        _layout_recursive(cmds, children, styles, child, child_content);

        if (row) {
            cursor_x += cw + cs.margin * 2.f + spacing;
        } else {
            cursor_y += ch + cs.margin * 2.f + spacing;
        }
    }
}

static void ui_compute_layout_system(
    r::ecs::Commands &cmds,
    r::ecs::Query<
        r::ecs::Optional<r::Parent>,
        r::ecs::Optional<r::Style>,
        r::ecs::Optional<r::Visibility>,
        r::ecs::Ref<r::UiNode>,
        r::ecs::EntityId
    > q,
    r::ecs::Res<r::WindowPluginConfig> win)
{
    std::unordered_map<r::ecs::Entity, std::vector<r::ecs::Entity>> children_map;
    std::unordered_map<r::ecs::Entity, r::Style> styles;
    std::unordered_set<r::ecs::Entity> present;
    struct Pending { r::ecs::Entity e; r::ecs::Entity parent; bool visible; };
    std::vector<Pending> pending;

    for (auto [parent_opt, style_opt, vis_opt, _node, id] : q) {
        const r::ecs::Entity e = id.value;
        present.insert(e);
        styles[e] = style_opt.ptr ? *style_opt.ptr : r::Style{};
        const bool visible = (!vis_opt.ptr || *vis_opt.ptr == r::Visibility::Visible);
        const r::ecs::Entity parent = (parent_opt.ptr ? parent_opt.ptr->id : 0);
        pending.push_back({e, parent, visible});
    }

    std::vector<r::ecs::Entity> roots;
    for (const auto &p : pending) {
        if (!p.visible) continue;
        if (p.parent == 0 || present.count(p.parent) == 0) {
            roots.push_back(p.e);
        } else {
            children_map[p.parent].push_back(p.e);
        }
    }

    const float ww = static_cast<float>(win.ptr->size.width);
    const float wh = static_cast<float>(win.ptr->size.height);

    for (auto root : roots) {
        const r::Style rs = styles[root];
        const float rw = (rs.width > 0.f) ? rs.width : ww;
        const float rh = (rs.height > 0.f) ? rs.height : wh;
        const float rx = 0.f;
        const float ry = 0.f;
        cmds.add_component(root, r::ComputedLayout{ rx, ry, rw, rh, rs.z_index });
        const Rect content{ rx + rs.padding, ry + rs.padding, rw - rs.padding * 2.f, rh - rs.padding * 2.f };
        _layout_recursive(cmds, children_map, styles, root, content);
    }
}

static void ui_render_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<r::Camera3d> cam,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>, r::ecs::Optional<r::Parent>, r::ecs::EntityId> q) noexcept
{
    EndMode3D();

    struct DrawItem {
        int z;
        size_t order;
        r::ecs::Entity id;
        const r::ComputedLayout *layout;
        r::Style style;
        r::ecs::Entity parent;
    };

    std::vector<DrawItem> items;
    items.reserve(128);

    std::unordered_map<r::ecs::Entity, const r::ComputedLayout *> layouts;
    std::unordered_map<r::ecs::Entity, r::Style> styles;
    std::unordered_map<r::ecs::Entity, r::ecs::Entity> parents;

    size_t ord = 0;
    for (auto [node, layout, style_opt, vis_opt, parent_opt, id] : q) {
        (void)node;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible)) continue;
        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        items.push_back(DrawItem{ s.z_index, ord++, id.value, layout.ptr, s, parent_opt.ptr ? parent_opt.ptr->id : 0 });
        layouts[id.value] = layout.ptr;
        styles[id.value] = s;
        parents[id.value] = parent_opt.ptr ? parent_opt.ptr->id : 0;
    }

    std::stable_sort(items.begin(), items.end(), [](const DrawItem &a, const DrawItem &b) {
        if (a.z != b.z) return a.z < b.z;
        return a.order < b.order;
    });

    auto intersect = [](int x, int y, int w, int h, int x2, int y2, int w2, int h2) {
        int nx = std::max(x, x2);
        int ny = std::max(y, y2);
        int r1 = x + w; int r2 = x2 + w2;
        int b1 = y + h; int b2 = y2 + h2;
        int nw = std::max(0, std::min(r1, r2) - nx);
        int nh = std::max(0, std::min(b1, b2) - ny);
        return ::Rectangle{ (float)nx, (float)ny, (float)nw, (float)nh };
    };

    for (const auto &it : items) {
        const int x = (int)it.layout->x;
        const int y = (int)it.layout->y;
        const int w = (int)it.layout->w;
        const int h = (int)it.layout->h;

        bool applied_scissor = false;
        ::Rectangle scissor = {0, 0, 0, 0};
        r::ecs::Entity p = it.parent;
        bool first = true;
        while (p != 0) {
            auto psit = styles.find(p);
            auto plit = layouts.find(p);
            if (psit != styles.end() && plit != layouts.end()) {
                const r::Style &ps = psit->second;
                const r::ComputedLayout *pl = plit->second;
                if (ps.clip_children) {
                    const int cx = (int)(pl->x + ps.padding);
                    const int cy = (int)(pl->y + ps.padding);
                    const int cw = (int)(pl->w - ps.padding * 2.f);
                    const int ch = (int)(pl->h - ps.padding * 2.f);
                    if (first) {
                        scissor = { (float)cx, (float)cy, (float)cw, (float)ch };
                        first = false;
                    } else {
                        scissor = intersect((int)scissor.x, (int)scissor.y, (int)scissor.width, (int)scissor.height,
                                            cx, cy, cw, ch);
                    }
                    applied_scissor = true;
                }
            }
            auto pit = parents.find(p);
            if (pit == parents.end()) break;
            p = pit->second;
        }

        if (applied_scissor) BeginScissorMode((int)scissor.x, (int)scissor.y, (int)scissor.width, (int)scissor.height);

        DrawRectangle(x, y, w, h, {it.style.background.r, it.style.background.g, it.style.background.b, it.style.background.a});
        if (it.style.border_thickness > 0.f) {
            ::Rectangle rec{ (float)x, (float)y, (float)w, (float)h };
            DrawRectangleLinesEx(rec, (int)it.style.border_thickness,
                { it.style.border_color.r, it.style.border_color.g, it.style.border_color.b, it.style.border_color.a });
        }

        if (applied_scissor) EndScissorMode();
    }
    if (cfg.ptr->show_debug_overlay) {
        DrawRectangle(8, 8, 220, 28, {255, 255, 255, 200});
        const char *text = cfg.ptr->overlay_text.c_str();
        const int font_size = 18;
        DrawText(text, 14, 12, font_size, {0, 0, 0, 255});
        DrawFPS(10, 40);
    }
    BeginMode3D(_to_raylib(*cam.ptr));
}

/**
 * PUBLIC UiPlugin
 */

UiPlugin::UiPlugin(const UiPluginConfig &cfg) noexcept : _config(cfg) {}

void UiPlugin::build(Application &app)
{
    app
        .insert_resource(_config)
        .insert_resource(UiFonts{})
        .insert_resource(UiTheme{})
        .insert_resource(UiInputState{})
        .insert_resource(UiEvents{})
        .add_systems(Schedule::STARTUP, ui_startup_system)
        .add_systems(Schedule::UPDATE, ui_remap_parents_system, ui_update_system, ui_compute_layout_system, ui_pointer_system)
        .add_systems(Schedule::RENDER, ui_render_system);

    Logger::info("UiPlugin built");
}

}
