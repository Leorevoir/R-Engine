#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace r::ui {

static float UI_THEME_SPACING = 8.f;
static float UI_THEME_PADDING = 8.f;

struct Rect { float x; float y; float w; float h; };

static void _layout_recursive(
    const std::unordered_map<u32, std::vector<u32>> &children,
    const std::unordered_map<u32, r::Style> &styles,
    std::unordered_map<u32, r::ComputedLayout*> &layouts,
    u32 node,
    const Rect &content)
{
    auto it = children.find(node);
    if (it == children.end()) return;

    const r::Style parent_style = [&]() { auto ps = styles.find(node); return (ps != styles.end()) ? ps->second : r::Style{}; }();

    const auto &kids = it->second;
    std::vector<u32> ordered_kids = kids;
    std::stable_sort(ordered_kids.begin(), ordered_kids.end(), [&](u32 a, u32 b){
        auto sa = styles.find(a); auto sb = styles.find(b);
        int oa = (sa != styles.end()) ? sa->second.order : 0;
        int ob = (sb != styles.end()) ? sb->second.order : 0;
        if (oa != ob) return oa < ob;
        int za = (sa != styles.end()) ? sa->second.z_index : 0;
        int zb = (sb != styles.end()) ? sb->second.z_index : 0;
        if (za != zb) return za < zb;
        return a < b;
    });
    const bool row = parent_style.direction == r::LayoutDirection::Row;
    const float main_size = row ? content.w : content.h;

    float total_main = 0.f;
    for (auto child : ordered_kids) {
        const r::Style cs = [&]() { auto f = styles.find(child); return (f != styles.end()) ? f->second : r::Style{}; }();
        const float cmargin = (cs.margin > 0.f) ? cs.margin : UI_THEME_SPACING;
        const float mw = (cs.width > 0.f) ? cs.width : 0.f;
        const float mh = (cs.height > 0.f) ? cs.height : 0.f;
        total_main += (row ? mw : mh) + cmargin * 2.f;
    }

    float free_space = main_size - total_main;
    if (free_space < 0.f) free_space = 0.f;

    float spacing = 0.f, lead = 0.f;
    const float gap = (parent_style.gap > 0.f) ? parent_style.gap : UI_THEME_SPACING;
    switch (parent_style.justify) {
        case r::JustifyContent::Start: lead = 0.f; spacing = 0.f; break;
        case r::JustifyContent::Center: lead = free_space * 0.5f; spacing = 0.f; break;
        case r::JustifyContent::End: lead = free_space; spacing = 0.f; break;
        case r::JustifyContent::SpaceBetween: spacing = (kids.size() > 1) ? (free_space / static_cast<float>(kids.size() - 1)) : 0.f; lead = 0.f; break;
        default: lead = 0.f; spacing = 0.f; break;
    }

    // Apply lead only along the main axis
    float cursor_x = content.x + (row ? lead : 0.f);
    float cursor_y = content.y + (row ? 0.f : lead);

    for (auto child : ordered_kids) {
        const r::Style cs = [&]() { auto f = styles.find(child); return (f != styles.end()) ? f->second : r::Style{}; }();
        const float cmargin = (cs.margin > 0.f) ? cs.margin : UI_THEME_SPACING;
        const float cpadding = (cs.padding > 0.f) ? cs.padding : UI_THEME_PADDING;

        float cw = cs.width;
        float ch = cs.height;
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
        if (cs.position == r::PositionType::Absolute) { x = content.x + cs.offset_x; y = content.y + cs.offset_y; }

        // Cross-axis alignment (Center/End) + AlignSelf override
        auto resolve_align = [&](const r::Style &child_style) -> r::AlignItems {
            if (child_style.align_self != r::AlignSelf::Auto) {
                switch (child_style.align_self) {
                    case r::AlignSelf::Start: return r::AlignItems::Start;
                    case r::AlignSelf::Center: return r::AlignItems::Center;
                    case r::AlignSelf::End: return r::AlignItems::End;
                    case r::AlignSelf::Stretch: return r::AlignItems::Stretch;
                    default: break;
                }
            }
            return parent_style.align;
        };

        const r::AlignItems eff_align = resolve_align(cs);
        if (cs.position == r::PositionType::Relative) {
            if (row) {
                // cross-axis is vertical
                switch (eff_align) {
                    case r::AlignItems::Center: y = content.y + (content.h - ch) * 0.5f; break;
                    case r::AlignItems::End:    y = content.y + content.h - ch - cmargin; break;
                    default: break; // Start already handled; Stretch handled above
                }
            } else {
                // Column: cross-axis is horizontal
                switch (eff_align) {
                    case r::AlignItems::Center: x = content.x + (content.w - cw) * 0.5f; break;
                    case r::AlignItems::End:    x = content.x + content.w - cw - cmargin; break;
                    default: break;
                }
            }
        }

        if (auto lit = layouts.find(child); lit != layouts.end() && lit->second) {
            *lit->second = r::ComputedLayout{ x, y, std::max(0.f, cw), std::max(0.f, ch), cs.z_index };
        }

        const Rect child_content{ x + cpadding, y + cpadding, cw - cpadding * 2.f, ch - cpadding * 2.f };
        _layout_recursive(children, styles, layouts, child, child_content);

        if (row && cs.position == r::PositionType::Relative) cursor_x += cw + cmargin * 2.f + spacing + gap;
        else if (cs.position == r::PositionType::Relative) cursor_y += ch + cmargin * 2.f + spacing + gap;
    }
}

void compute_layout_system(
    r::ecs::Query<
        r::ecs::Mut<r::ComputedLayout>,
        r::ecs::Optional<r::Style>,
        r::ecs::Optional<r::Visibility>,
        r::ecs::Optional<r::UiParent>,
        r::ecs::Optional<r::UiScroll>,
        r::ecs::Optional<r::UiId>
    > q,
    r::ecs::Res<r::UiTheme> theme)
{
    UI_THEME_SPACING = theme.ptr->spacing;
    UI_THEME_PADDING = (float)theme.ptr->padding;

    std::unordered_map<u32, std::vector<u32>> children_map;
    std::unordered_map<u32, r::Style> styles;
    std::unordered_map<u32, r::ComputedLayout*> layouts;
    std::unordered_set<u32> present;

    struct NodeInfo { u32 handle; u32 parent; bool visible; };
    std::vector<NodeInfo> nodes;

    for (auto [layout, style_opt, vis_opt, parent_opt, scroll_opt, id_opt] : q) {
        (void)scroll_opt;
        const u32 h = id_opt.ptr ? id_opt.ptr->value : 0u;
        const u32 p = parent_opt.ptr ? parent_opt.ptr->handle : 0u;
        present.insert(h);
        styles[h] = style_opt.ptr ? *style_opt.ptr : r::Style{};
        layouts[h] = layout.ptr;
        const bool visible = (!vis_opt.ptr || *vis_opt.ptr == r::Visibility::Visible);
        nodes.push_back(NodeInfo{h, p, visible});
    }

    std::vector<u32> roots;
    for (const auto &n : nodes) {
        if (!n.visible) continue;
        if (n.parent == 0 || present.count(n.parent) == 0) roots.push_back(n.handle);
        else children_map[n.parent].push_back(n.handle);
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
        const float rx = 0.f;
        const float ry = 0.f;
        if (auto lit = layouts.find(root); lit != layouts.end() && lit->second) {
            *lit->second = r::ComputedLayout{ rx, ry, rw, rh, rs.z_index };
        }
        const float root_padding = (rs.padding > 0.f) ? rs.padding : (float)theme.ptr->padding;
        const Rect content{ rx + root_padding, ry + root_padding, rw - root_padding * 2.f, rh - root_padding * 2.f };
        _layout_recursive(children_map, styles, layouts, root, content);
    }
}

void scroll_clamp_system(
    r::ecs::Query<r::ecs::Mut<r::UiScroll>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::UiId>> scq,
    r::ecs::Query<r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::UiParent>, r::ecs::Optional<r::UiId>> allq,
    r::ecs::Res<r::UiTheme> theme)
{
    (void)theme;
    std::unordered_set<u32> containers;
    std::unordered_map<u32, const r::ComputedLayout *> cont_layout;
    std::unordered_map<u32, float> cont_pad;

    for (auto [scroll, layout, id_opt] : scq) {
        (void)scroll;
        const u32 h = id_opt.ptr ? id_opt.ptr->value : 0u;
        if (h == 0) continue;
        containers.insert(h);
        cont_layout[h] = layout.ptr;
    }

    for (auto [layout, style_opt, parent_opt, id_opt] : allq) {
        const u32 h = id_opt.ptr ? id_opt.ptr->value : 0u;
        if (containers.find(h) != containers.end()) {
            const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
            cont_pad[h] = (s.padding > 0.f) ? s.padding : UI_THEME_PADDING;
        }
    }

    std::unordered_map<u32, float> content_bottom;
    for (const auto &kv : containers) {
        const auto *pl = cont_layout[kv];
        const float pad = cont_pad[kv];
        content_bottom[kv] = pl->y + pad; /* initial top */
    }

    for (auto [layout, style_opt, parent_opt, id_opt] : allq) {
        const u32 ph = parent_opt.ptr ? parent_opt.ptr->handle : 0u;
        if (containers.find(ph) == containers.end()) continue;
        const float bottom = layout.ptr->y + layout.ptr->h;
        auto it = content_bottom.find(ph);
        if (it != content_bottom.end()) it->second = std::max(it->second, bottom);
    }

    for (auto [scroll, layout, id_opt] : scq) {
        const u32 e = id_opt.ptr ? id_opt.ptr->value : 0u;
        if (e == 0) continue;
        const float pad = cont_pad[e];
        const float viewport = layout.ptr->h - pad * 2.f;
        float content_h = std::max(0.f, content_bottom[e] - (layout.ptr->y + pad));
        const float max_scroll = std::max(0.f, content_h - viewport);
        if (scroll.ptr->y < 0.f) scroll.ptr->y = 0.f;
        if (scroll.ptr->y > max_scroll) scroll.ptr->y = max_scroll;
    }
}

} // namespace r::ui
