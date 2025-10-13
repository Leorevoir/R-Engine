/**
 * \file UiSystems.Layout.cpp
 * \brief Layout computation systems for the UI plugin.
 */
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace r::ui {

static float UI_THEME_SPACING = 8.f;
static float UI_THEME_PADDING = 8.f;

struct Rect {
        float x;
        float y;
        float w;
        float h;
};

static void _layout_recursive(const std::unordered_map<r::ecs::Entity, std::vector<r::ecs::Entity>> &children,
    const std::unordered_map<r::ecs::Entity, r::Style> &styles, std::unordered_map<r::ecs::Entity, r::ComputedLayout *> &layouts,
    r::ecs::Entity node, const Rect &content)
{
    auto it = children.find(node);
    if (it == children.end())
        return;

    const r::Style parent_style = [&]() {
        auto ps = styles.find(node);
        return (ps != styles.end()) ? ps->second : r::Style{};
    }();

    const auto &kids = it->second;
    std::vector<r::ecs::Entity> ordered_kids = kids;
    std::stable_sort(ordered_kids.begin(), ordered_kids.end(), [&](r::ecs::Entity a, r::ecs::Entity b) {
        auto sa = styles.find(a);
        auto sb = styles.find(b);
        int oa = (sa != styles.end()) ? sa->second.order : 0;
        int ob = (sb != styles.end()) ? sb->second.order : 0;
        if (oa != ob)
            return oa < ob;
        int za = (sa != styles.end()) ? sa->second.z_index : 0;
        int zb = (sb != styles.end()) ? sb->second.z_index : 0;
        if (za != zb)
            return za < zb;
        return a < b;
    });
    const bool row = parent_style.direction == r::LayoutDirection::Row;
    const float main_size = row ? content.w : content.h;

    float total_main = 0.f;
    for (auto child : ordered_kids) {
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
    if (free_space < 0.f)
        free_space = 0.f;

    float spacing = 0.f, lead = 0.f;
    const float gap = (parent_style.gap > 0.f) ? parent_style.gap : UI_THEME_SPACING;
    switch (parent_style.justify) {
        case r::JustifyContent::Start:
            lead = 0.f;
            spacing = 0.f;
            break;
        case r::JustifyContent::Center:
            lead = free_space * 0.5f;
            spacing = 0.f;
            break;
        case r::JustifyContent::End:
            lead = free_space;
            spacing = 0.f;
            break;
        case r::JustifyContent::SpaceBetween:
            spacing = (kids.size() > 1) ? (free_space / static_cast<float>(kids.size() - 1)) : 0.f;
            lead = 0.f;
            break;
        default:
            lead = 0.f;
            spacing = 0.f;
            break;
    }

    /* Apply lead only along the main axis */
    float cursor_x = content.x + (row ? lead : 0.f);
    float cursor_y = content.y + (row ? 0.f : lead);

    for (auto child : ordered_kids) {
        const r::Style cs = [&]() {
            auto f = styles.find(child);
            return (f != styles.end()) ? f->second : r::Style{};
        }();
        const float cmargin = (cs.margin > 0.f) ? cs.margin : UI_THEME_SPACING;
        const float cpadding = (cs.padding > 0.f) ? cs.padding : UI_THEME_PADDING;

        float cw = cs.width;
        float ch = cs.height;
        if (cs.width_pct >= 0.f)
            cw = content.w * (cs.width_pct / 100.f);
        if (cs.height_pct >= 0.f)
            ch = content.h * (cs.height_pct / 100.f);
        if (cw <= 0.f)
            cw = row ? 0.f : content.w - cmargin * 2.f;
        if (ch <= 0.f)
            ch = row ? content.h - cmargin * 2.f : 0.f;
        if (cs.min_width > 0.f)
            cw = std::max(cw, cs.min_width);
        if (cs.max_width > 0.f)
            cw = std::min(cw, cs.max_width);
        if (cs.min_height > 0.f)
            ch = std::max(ch, cs.min_height);
        if (cs.max_height > 0.f)
            ch = std::min(ch, cs.max_height);

        if (parent_style.align == r::AlignItems::Stretch) {
            if (row && cs.height <= 0.f)
                ch = content.h - cs.margin * 2.f;
            if (!row && cs.width <= 0.f)
                cw = content.w - cs.margin * 2.f;
        }

        float x = row ? (cursor_x + cmargin) : (content.x + cmargin);
        float y = row ? (content.y + cmargin) : (cursor_y + cmargin);
        if (cs.position == r::PositionType::Absolute) {
            x = content.x + cs.offset_x;
            y = content.y + cs.offset_y;
        }

        /* Cross-axis alignment (Center/End) + AlignSelf override */
        auto resolve_align = [&](const r::Style &child_style) -> r::AlignItems {
            if (child_style.align_self != r::AlignSelf::Auto) {
                switch (child_style.align_self) {
                    case r::AlignSelf::Start:
                        return r::AlignItems::Start;
                    case r::AlignSelf::Center:
                        return r::AlignItems::Center;
                    case r::AlignSelf::End:
                        return r::AlignItems::End;
                    case r::AlignSelf::Stretch:
                        return r::AlignItems::Stretch;
                    default:
                        break;
                }
            }
            return parent_style.align;
        };

        const r::AlignItems eff_align = resolve_align(cs);
        if (cs.position == r::PositionType::Relative) {
            if (row) {
                /* cross-axis is vertical */
                switch (eff_align) {
                    case r::AlignItems::Center:
                        y = content.y + (content.h - ch) * 0.5f;
                        break;
                    case r::AlignItems::End:
                        y = content.y + content.h - ch - cmargin;
                        break;
                    default:
                        break;
                }
            } else {
                /* Column: cross-axis is horizontal */
                switch (eff_align) {
                    case r::AlignItems::Center:
                        x = content.x + (content.w - cw) * 0.5f;
                        break;
                    case r::AlignItems::End:
                        x = content.x + content.w - cw - cmargin;
                        break;
                    default:
                        break;
                }
            }
        }

        if (auto lit = layouts.find(child); lit != layouts.end() && lit->second) {
            *lit->second = r::ComputedLayout{x, y, std::max(0.f, cw), std::max(0.f, ch), cs.z_index};
        }

        const Rect child_content{x + cpadding, y + cpadding, cw - cpadding * 2.f, ch - cpadding * 2.f};
        _layout_recursive(children, styles, layouts, child, child_content);

        if (row && cs.position == r::PositionType::Relative)
            cursor_x += cw + cmargin * 2.f + spacing + gap;
        else if (cs.position == r::PositionType::Relative)
            cursor_y += ch + cmargin * 2.f + spacing + gap;
    }
}

void compute_layout_system(r::ecs::Query<r::ecs::Mut<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>,
                               r::ecs::Optional<r::ecs::Parent>, r::ecs::Optional<r::UiScroll>, r::ecs::Optional<r::ecs::Children>>
                               q,
    r::ecs::Res<r::UiTheme> theme)
{
    UI_THEME_SPACING = theme.ptr->spacing;
    UI_THEME_PADDING = static_cast<float>(theme.ptr->padding);

    std::unordered_map<r::ecs::Entity, std::vector<r::ecs::Entity>> children_map;
    std::unordered_map<r::ecs::Entity, r::Style> styles;
    std::unordered_map<r::ecs::Entity, r::ComputedLayout *> layouts;
    std::unordered_set<r::ecs::Entity> present;
    std::unordered_map<r::ecs::Entity, r::ecs::Entity> parent_from_children;

    struct NodeInfo {
            r::ecs::Entity id;
            r::ecs::Entity parent;
            bool visible;
    };
    std::vector<NodeInfo> nodes;

    for (auto it = q.begin(); it != q.end(); ++it) {
        auto [layout, style_opt, vis_opt, parent_opt, scroll_opt, children_opt] = *it;
        (void) scroll_opt;
        const auto id = static_cast<r::ecs::Entity>(it.entity());
        const auto p = parent_opt.ptr ? parent_opt.ptr->entity : r::ecs::NULL_ENTITY;
        present.insert(id);
        styles[id] = style_opt.ptr ? *style_opt.ptr : r::Style{};
        layouts[id] = layout.ptr;
        const bool visible = (!vis_opt.ptr || *vis_opt.ptr == r::Visibility::Visible);
        nodes.push_back(NodeInfo{id, p, visible});
        if (children_opt.ptr) {
            for (auto child : children_opt.ptr->entities) {
                parent_from_children[child] = id;
            }
        }
    }

    constexpr auto PLACEHOLDER = std::numeric_limits<r::ecs::Entity>::max();
    for (auto &node : nodes) {
        auto it = parent_from_children.find(node.id);
        if (it != parent_from_children.end()) {
            node.parent = it->second;
        } else if (node.parent == PLACEHOLDER) {
            node.parent = r::ecs::NULL_ENTITY;
        }
    }

    std::vector<r::ecs::Entity> roots;
    for (const auto &n : nodes) {
        if (!n.visible)
            continue;
        if (n.parent == r::ecs::NULL_ENTITY || present.count(n.parent) == 0)
            roots.push_back(n.id);
        else
            children_map[n.parent].push_back(n.id);
    }

    const float ww = static_cast<float>(GetRenderWidth());
    const float wh = static_cast<float>(GetRenderHeight());
    for (auto root : roots) {
        const r::Style rs = styles[root];
        float rw = (rs.width_pct >= 0.f) ? (ww * (rs.width_pct / 100.f)) : ((rs.width > 0.f) ? rs.width : ww);
        float rh = (rs.height_pct >= 0.f) ? (wh * (rs.height_pct / 100.f)) : ((rs.height > 0.f) ? rs.height : wh);
        if (rs.min_width > 0.f)
            rw = std::max(rw, rs.min_width);
        if (rs.max_width > 0.f)
            rw = std::min(rw, rs.max_width);
        if (rs.min_height > 0.f)
            rh = std::max(rh, rs.min_height);
        if (rs.max_height > 0.f)
            rh = std::min(rh, rs.max_height);
        const float rx = 0.f;
        const float ry = 0.f;
        if (auto lit = layouts.find(root); lit != layouts.end() && lit->second) {
            *lit->second = r::ComputedLayout{rx, ry, rw, rh, rs.z_index};
        }
        const float root_padding = (rs.padding > 0.f) ? rs.padding : static_cast<float>(theme.ptr->padding);
        const Rect content{rx + root_padding, ry + root_padding, rw - root_padding * 2.f, rh - root_padding * 2.f};
        _layout_recursive(children_map, styles, layouts, root, content);
    }
}

void scroll_clamp_system(r::ecs::Query<r::ecs::Mut<r::UiScroll>, r::ecs::Ref<r::ComputedLayout>> scq,
    r::ecs::Query<r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::ecs::Parent>,
        r::ecs::Optional<r::ecs::Children>>
        allq,
    r::ecs::Res<r::UiTheme> theme)
{
    (void) theme;
    std::unordered_set<r::ecs::Entity> containers;
    std::unordered_map<r::ecs::Entity, const r::ComputedLayout *> cont_layout;
    std::unordered_map<r::ecs::Entity, float> cont_pad;

    for (auto it = scq.begin(); it != scq.end(); ++it) {
        auto [scroll, layout] = *it;
        (void) scroll;
        const auto id = static_cast<r::ecs::Entity>(it.entity());
        containers.insert(id);
        cont_layout[id] = layout.ptr;
    }

    for (auto it = allq.begin(); it != allq.end(); ++it) {
        auto [layout, style_opt, parent_opt, children_opt] = *it;
        (void) layout;
        (void) parent_opt;
        (void) children_opt;
        const auto id = static_cast<r::ecs::Entity>(it.entity());
        if (containers.find(id) != containers.end()) {
            const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
            cont_pad[id] = (s.padding > 0.f) ? s.padding : UI_THEME_PADDING;
        }
    }

    std::unordered_map<r::ecs::Entity, float> content_bottom;
    for (const auto &kv : containers) {
        const auto *pl = cont_layout[kv];
        const float pad = cont_pad[kv];
        content_bottom[kv] = pl->y + pad; /* initial top */
    }

    std::unordered_map<r::ecs::Entity, r::ecs::Entity> parent_from_children;
    for (auto it = allq.begin(); it != allq.end(); ++it) {
        auto [layout, style_opt, parent_opt, children_opt] = *it;
        (void) layout;
        (void) style_opt;
        (void) parent_opt;
        if (children_opt.ptr) {
            const auto id = static_cast<r::ecs::Entity>(it.entity());
            for (auto child : children_opt.ptr->entities)
                parent_from_children[child] = id;
        }
    }

    for (auto it = allq.begin(); it != allq.end(); ++it) {
        auto [layout, style_opt, parent_opt, children_opt] = *it;
        (void) style_opt;
        (void) children_opt;
        auto ph = parent_opt.ptr ? parent_opt.ptr->entity : r::ecs::NULL_ENTITY;
        if (auto pit = parent_from_children.find(static_cast<r::ecs::Entity>(it.entity())); pit != parent_from_children.end()) {
            ph = pit->second;
        }
        if (containers.find(ph) == containers.end())
            continue;
        const float bottom = layout.ptr->y + layout.ptr->h;
        auto it2 = content_bottom.find(ph);
        if (it2 != content_bottom.end())
            it2->second = std::max(it2->second, bottom);
    }

    for (auto it = scq.begin(); it != scq.end(); ++it) {
        auto [scroll, layout] = *it;
        const auto e = static_cast<r::ecs::Entity>(it.entity());
        const float pad = cont_pad[e];
        const float viewport = layout.ptr->h - pad * 2.f;
        float content_h = std::max(0.f, content_bottom[e] - (layout.ptr->y + pad));
        const float max_scroll = std::max(0.f, content_h - viewport);
        if (scroll.ptr->y < 0.f)
            scroll.ptr->y = 0.f;
        if (scroll.ptr->y > max_scroll)
            scroll.ptr->y = max_scroll;
    }
}

}// namespace r::ui
