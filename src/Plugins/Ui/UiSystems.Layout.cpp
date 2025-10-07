#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace r::ui {

static float UI_THEME_SPACING = 8.f;
static float UI_THEME_PADDING = 8.f;

struct Rect { float x; float y; float w; float h; };

static void _layout_recursive(r::ecs::Commands &cmds,
    const std::unordered_map<r::ecs::Entity, std::vector<r::ecs::Entity>> &children,
    const std::unordered_map<r::ecs::Entity, r::Style> &styles,
    r::ecs::Entity node,
    const Rect &content)
{
    auto it = children.find(node);
    if (it == children.end()) return;

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
        const float cmargin = (cs.margin > 0.f) ? cs.margin : UI_THEME_SPACING;
        const float mw = (cs.width > 0.f) ? cs.width : 0.f;
        const float mh = (cs.height > 0.f) ? cs.height : 0.f;
        total_main += (row ? mw : mh) + cmargin * 2.f;
    }

    float free_space = main_size - total_main;
    if (free_space < 0.f) free_space = 0.f;

    float spacing = 0.f;
    float lead = 0.f;
    const float gap = (parent_style.gap > 0.f) ? parent_style.gap : UI_THEME_SPACING;
    switch (parent_style.justify) {
        case r::JustifyContent::Start: lead = 0.f; spacing = 0.f; break;
        case r::JustifyContent::Center: lead = free_space * 0.5f; spacing = 0.f; break;
        case r::JustifyContent::End: lead = free_space; spacing = 0.f; break;
        case r::JustifyContent::SpaceBetween:
            spacing = (kids.size() > 1) ? (free_space / static_cast<float>(kids.size() - 1)) : 0.f;
            lead = 0.f; break;
        default: lead = 0.f; spacing = 0.f; break;
    }

    float cursor_x = content.x + lead;
    float cursor_y = content.y + lead;

    for (auto child : kids) {
        const r::Style cs = [&]() {
            auto f = styles.find(child);
            return (f != styles.end()) ? f->second : r::Style{};
        }();

        const float cmargin = (cs.margin > 0.f) ? cs.margin : UI_THEME_SPACING;
        const float cpadding = (cs.padding > 0.f) ? cs.padding : UI_THEME_PADDING;

        float cw = cs.width; float ch = cs.height;
        if (cs.width_pct >= 0.f) cw = content.w * (cs.width_pct / 100.f);
        if (cs.height_pct >= 0.f) ch = content.h * (cs.height_pct / 100.f);
        if (cw <= 0.f) cw = row ? 0.f : content.w - cmargin * 2.f;
        if (ch <= 0.f) ch = row ? content.h - cmargin * 2.f : 0.f;
        if (cs.min_width > 0.f) cw = std::max(cw, cs.min_width);
        if (cs.max_width > 0.f) cw = std::min(cw, cs.max_width);
        if (cs.min_height > 0.f) ch = std::max(ch, cs.min_height);
        if (cs.max_height > 0.f) ch = std::min(ch, cs.max_height);

        if (parent_style.align == r::AlignItems::Stretch) {
            if (row && cs.height <= 0.f) ch = content.h - cs.margin * 2.f;
            if (!row && cs.width <= 0.f) cw = content.w - cs.margin * 2.f;
        }

        float x = row ? (cursor_x + cmargin) : (content.x + cmargin);
        float y = row ? (content.y + cmargin) : (cursor_y + cmargin);

        if (cs.position == r::PositionType::Absolute) {
            x = content.x + cs.offset_x;
            y = content.y + cs.offset_y;
        }

        const float px = x;
        const float py = y;
        const float pw = std::max(0.f, cw);
        const float ph = std::max(0.f, ch);
        cmds.add_component(child, r::ComputedLayout{ px, py, pw, ph, cs.z_index });

        const Rect child_content{ x + cpadding, y + cpadding, cw - cpadding * 2.f, ch - cpadding * 2.f };
        _layout_recursive(cmds, children, styles, child, child_content);

        if (row && cs.position == r::PositionType::Relative) cursor_x += cw + cmargin * 2.f + spacing + gap;
        else if (cs.position == r::PositionType::Relative)   cursor_y += ch + cmargin * 2.f + spacing + gap;
    }
}

void compute_layout_system(
    r::ecs::Commands &cmds,
    r::ecs::Query<
        r::ecs::Optional<r::Parent>,
        r::ecs::Optional<r::Style>,
        r::ecs::Optional<r::Visibility>,
        r::ecs::Ref<r::UiNode>,
        r::ecs::EntityId
    > q,
    r::ecs::Res<r::WindowPluginConfig> /*win*/,
    r::ecs::Res<r::UiTheme> theme)
{
    UI_THEME_SPACING = theme.ptr->spacing;
    UI_THEME_PADDING = static_cast<float>(theme.ptr->padding);
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

    const float ww = (float)GetRenderWidth();
    const float wh = (float)GetRenderHeight();

    for (auto root : roots) {
        const r::Style rs = styles[root];
        float rw = (rs.width_pct >= 0.f) ? (ww * (rs.width_pct / 100.f)) : ((rs.width > 0.f) ? rs.width : ww);
        float rh = (rs.height_pct >= 0.f) ? (wh * (rs.height_pct / 100.f)) : ((rs.height > 0.f) ? rs.height : wh);
        if (rs.min_width > 0.f) rw = std::max(rw, rs.min_width);
        if (rs.max_width > 0.f) rw = std::min(rw, rs.max_width);
        if (rs.min_height > 0.f) rh = std::max(rh, rs.min_height);
        if (rs.max_height > 0.f) rh = std::min(rh, rs.max_height);
        const float rx = 0.f; const float ry = 0.f;
        cmds.add_component(root, r::ComputedLayout{ rx, ry, rw, rh, rs.z_index });
        const float root_padding = (rs.padding > 0.f) ? rs.padding : (float)theme.ptr->padding;
        const Rect content{ rx + root_padding, ry + root_padding, rw - root_padding * 2.f, rh - root_padding * 2.f };
        _layout_recursive(cmds, children_map, styles, root, content);
    }
}

void scroll_clamp_system(
    r::ecs::Query<r::ecs::Mut<r::UiScroll>, r::ecs::Ref<r::ComputedLayout>, r::ecs::EntityId> scq,
    r::ecs::Query<r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Parent>, r::ecs::EntityId> allq,
    r::ecs::Res<r::UiTheme> theme)
{
    std::unordered_set<r::ecs::Entity> containers;
    std::unordered_map<r::ecs::Entity, const r::ComputedLayout *> cont_layout;
    std::unordered_map<r::ecs::Entity, float> cont_pad;

    for (auto [scroll, layout, id] : scq) {
        (void)scroll;
        containers.insert(id.value);
        cont_layout[id.value] = layout.ptr;
    }

    for (auto [layout, style_opt, parent_opt, id] : allq) {
        (void)parent_opt;
        if (containers.find(id.value) != containers.end()) {
            const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
            cont_pad[id.value] = (s.padding > 0.f) ? s.padding : (float)theme.ptr->padding;
        }
    }

    std::unordered_map<r::ecs::Entity, float> content_bottom;
    for (const auto &kv : containers) {
        const auto *pl = cont_layout[kv];
        const float pad = cont_pad[kv];
        content_bottom[kv] = pl->y + pad; /* initial top */
    }

    for (auto [layout, style_opt, parent_opt, id] : allq) {
        if (!parent_opt.ptr) continue;
        const auto p = parent_opt.ptr->id;
        if (containers.find(p) == containers.end()) continue;
        const float bottom = layout.ptr->y + layout.ptr->h;
        auto it = content_bottom.find(p);
        if (it != content_bottom.end()) it->second = std::max(it->second, bottom);
    }

    for (auto [scroll, layout, id] : scq) {
        const auto e = id.value;
        const float pad = cont_pad[e];
        const float viewport = layout.ptr->h - pad * 2.f;
        float content_h = std::max(0.f, content_bottom[e] - (layout.ptr->y + pad));
        const float max_scroll = std::max(0.f, content_h - viewport);
        if (scroll.ptr->y < 0.f) scroll.ptr->y = 0.f;
        if (scroll.ptr->y > max_scroll) scroll.ptr->y = max_scroll;
    }
}

} // namespace r::ui

