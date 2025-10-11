/**
 * \file UiSystems.Render.cpp
 * \brief Rendering system for the UI plugin using Raylib.
 */
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <R-Engine/ECS/Command.hpp>

namespace r::ui {

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

void render_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<r::Camera3d> cam,
    r::ecs::Res<r::UiInputState> input,
    r::ecs::Res<r::UiTheme> theme,
    r::ecs::ResMut<r::UiTextures> textures,
    r::ecs::ResMut<r::UiFonts> fonts,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>, r::ecs::Optional<r::ecs::Parent>, r::ecs::Optional<r::UiText>, r::ecs::Optional<r::UiImage>, r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::UiScroll>> q) noexcept
{
    EndMode3D();

    struct DrawItem { int z; size_t order; u32 handle; const r::ComputedLayout *layout; r::Style style; u32 parent; bool is_button; bool is_disabled; };

    std::vector<DrawItem> items; items.reserve(128);
    std::unordered_map<u32, const r::UiScroll*> scrolls; std::unordered_map<u32, float> content_bottom;
    std::unordered_map<u32, const r::ComputedLayout *> layouts; std::unordered_map<u32, r::Style> styles; std::unordered_map<u32, u32> parents;

    size_t ord = 0;
    for (auto it = q.begin(); it != q.end(); ++it) {
        auto [node, layout, style_opt, vis_opt, parent_opt, text_opt, image_opt, button_opt, scroll_opt] = *it;
        (void)node; (void)text_opt; (void)image_opt;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible)) continue;
        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        const bool is_button = button_opt.ptr != nullptr;
        const bool is_disabled = (button_opt.ptr && button_opt.ptr->disabled);
        const u32 id = static_cast<u32>(it.entity());
        const u32 pid = parent_opt.ptr ? static_cast<u32>(parent_opt.ptr->entity) : 0u;
        items.push_back({ s.z_index, ord++, id, layout.ptr, s, pid, is_button, is_disabled });
        layouts[id] = layout.ptr; styles[id] = s; parents[id] = pid; if (scroll_opt.ptr) scrolls[id] = scroll_opt.ptr;
    }

    std::stable_sort(items.begin(), items.end(), [](const DrawItem &a, const DrawItem &b) { if (a.z != b.z) return a.z < b.z; return a.order < b.order; });

    auto intersect = [](int x, int y, int w, int h, int x2, int y2, int w2, int h2) {
        int nx = std::max(x, x2); int ny = std::max(y, y2); int r1 = x + w; int r2 = x2 + w2; int b1 = y + h; int b2 = y2 + h2;
        int nw = std::max(0, std::min(r1, r2) - nx); int nh = std::max(0, std::min(b1, b2) - ny);
        return ::Rectangle{ static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nw), static_cast<float>(nh) };
    };

    auto scroll_of = [&](u32 e){ float sx=0.f, sy=0.f; u32 p = e; while (p!=0){ auto pit=parents.find(p); if (pit==parents.end()) break; p = pit->second; auto sit=styles.find(p); auto scit = scrolls.find(p); if (sit!=styles.end() && (sit->second.clip_children || sit->second.overflow_clip) && scit!=scrolls.end()) { sx -= scit->second->x; sy -= scit->second->y; } } return std::pair<float,float>{sx,sy}; };

    for (const auto &it : items) {
        auto p_it = parents.find(it.handle);
        if (p_it != parents.end()) {
            auto parent_id = p_it->second;
            if (scrolls.find(parent_id) != scrolls.end()) {
                float bottom = it.layout->y + it.layout->h;
                content_bottom[parent_id] = std::max(content_bottom[parent_id], bottom);
            }
        }
    }

    for (const auto &it : items) {
        auto ss = scroll_of(it.handle);
        const float sx = ss.first; const float sy = ss.second;
        const int x = static_cast<int>(it.layout->x); const int y = static_cast<int>(it.layout->y); const int w = static_cast<int>(it.layout->w); const int h = static_cast<int>(it.layout->h);

        bool applied_scissor = false; ::Rectangle scissor = {0, 0, 0, 0};
        u32 p = it.parent; bool first = true;
        while (p != 0) {
            auto psit = styles.find(p); auto plit = layouts.find(p);
            if (psit != styles.end() && plit != layouts.end()) {
                const r::Style &ps = psit->second; const r::ComputedLayout *pl = plit->second;
                if (ps.clip_children) {
                    const int cx = static_cast<int>(pl->x + ps.padding); const int cy = static_cast<int>(pl->y + ps.padding);
                    const int cw = static_cast<int>(pl->w - ps.padding * 2.f); const int ch = static_cast<int>(pl->h - ps.padding * 2.f);
                    if (first) { scissor = { static_cast<float>(cx), static_cast<float>(cy), static_cast<float>(cw), static_cast<float>(ch) }; first = false; }
                    else { scissor = intersect(static_cast<int>(scissor.x), static_cast<int>(scissor.y), static_cast<int>(scissor.width), static_cast<int>(scissor.height), cx, cy, cw, ch); }
                    applied_scissor = true;
                }
            }
            auto pit = parents.find(p); if (pit == parents.end()) break; p = pit->second;
        }

        if (applied_scissor) BeginScissorMode(static_cast<int>(scissor.x), static_cast<int>(scissor.y), static_cast<int>(scissor.width), static_cast<int>(scissor.height));

        r::Color bg = it.style.background; r::Color border = it.style.border_color; float border_thickness = it.style.border_thickness;

        if (it.is_button) {
            if (it.is_disabled) { bg = theme.ptr->button.bg_disabled; border = theme.ptr->button.border_disabled; border_thickness = theme.ptr->button.border_thickness; }
            else if (input.ptr->active == it.handle) { bg = theme.ptr->button.bg_pressed; border = theme.ptr->button.border_pressed; border_thickness = theme.ptr->button.border_thickness; }
            else if (input.ptr->hovered == it.handle) { bg = theme.ptr->button.bg_hover; border = theme.ptr->button.border_hover; border_thickness = theme.ptr->button.border_thickness; }
            else { bg = theme.ptr->button.bg_normal; border = theme.ptr->button.border_normal; border_thickness = theme.ptr->button.border_thickness; }
        }
        if (!it.is_button && bg.a == 0) { bg = theme.ptr->panel_bg; }

        DrawRectangle(static_cast<int>(static_cast<float>(x) + sx), static_cast<int>(static_cast<float>(y) + sy), w, h, {bg.r, bg.g, bg.b, bg.a});
        if (border_thickness > 0.f) {
            ::Rectangle rec{ static_cast<float>(x) + sx, static_cast<float>(y) + sy, static_cast<float>(w), static_cast<float>(h) };
            DrawRectangleLinesEx(rec, static_cast<float>(border_thickness), { border.r, border.g, border.b, border.a });
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

    /* Debug bounds and highlights */
    if (cfg.ptr->debug_draw_bounds) {
        for (auto it = q.begin(); it != q.end(); ++it) {
            auto [node, layout, style_opt, vis_opt, parent_opt, text_opt, image_opt, button_opt, scroll_opt] = *it;
            (void)node; (void)style_opt; (void)vis_opt; (void)text_opt; (void)image_opt; (void)button_opt; (void)scroll_opt; (void)parent_opt;
            const int x = static_cast<int>(layout.ptr->x); const int y = static_cast<int>(layout.ptr->y); const int w = static_cast<int>(layout.ptr->w); const int h = static_cast<int>(layout.ptr->h);
            DrawRectangleLines(x, y, w, h, {120,120,120,120});
        }
        auto draw_highlight = [&](u32 h, ::Color c){ if (h==0) return; for (auto it = q.begin(); it != q.end(); ++it){ auto [node, layout, sopt, vopt, popt, topt, iopt, bopt, scopt] = *it; (void)node;(void)sopt;(void)vopt;(void)popt;(void)topt;(void)iopt;(void)bopt;(void)scopt; u32 eid = static_cast<u32>(it.entity()); if (eid==h){ ::Rectangle r{layout.ptr->x, layout.ptr->y, layout.ptr->w, layout.ptr->h}; DrawRectangleLinesEx(r, 2, c); break; } } };
        draw_highlight(input.ptr->hovered, {0,255,0,200});
        draw_highlight(input.ptr->active, {255,165,0,200});
        draw_highlight(input.ptr->focused, {255,255,0,200});
    }

    /* Scrollbars */
    for (const auto &kv : scrolls) {
        u32 cont = kv.first; const auto *pl = layouts[cont]; const r::Style ps = styles[cont];
        const float pad = (ps.padding > 0.f) ? ps.padding : static_cast<float>(theme.ptr->padding);
        const float viewport_h = pl->h - pad * 2.f;
        const float content_h = std::max(0.f, content_bottom[cont] - (pl->y + pad));
        if (content_h <= viewport_h + 1.f) continue;
        const float max_scroll = content_h - viewport_h; const float scroll_y = std::min(std::max(0.f, kv.second->y), max_scroll);
        const int track_x = static_cast<int>(pl->x + pl->w - 6); const int track_y = static_cast<int>(pl->y + pad); const int track_w = 4; const int track_h = static_cast<int>(viewport_h);
        DrawRectangle(track_x, track_y, track_w, track_h, {70,70,70,180});
        const int thumb_h = std::max(20, static_cast<int>(viewport_h * (viewport_h / content_h)));
        const int thumb_y = track_y + static_cast<int>((viewport_h - static_cast<float>(thumb_h)) * (scroll_y / max_scroll));
        DrawRectangle(track_x, thumb_y, track_w, thumb_h, {200,200,200,220});
        DrawRectangleLines(track_x, track_y, track_w, track_h, {220,220,220,120});
    }

    /* Foreground: images and text */
    std::unordered_map<u32, const r::ComputedLayout *> layouts2; layouts2 = layouts;
    std::unordered_map<u32, r::Style> styles2; styles2 = styles;
    std::unordered_map<u32, u32> parents2; parents2 = parents;

    auto scroll_of2 = [&](u32 e){ float sx=0.f, sy=0.f; u32 p = e; while (p!=0){ auto pit=parents2.find(p); if (pit==parents2.end()) break; p = pit->second; auto sit=styles2.find(p); auto scit = scrolls.find(p); if (sit!=styles2.end() && (sit->second.clip_children || sit->second.overflow_clip) && scit!=scrolls.end()) { sx -= scit->second->x; sy -= scit->second->y; } } return std::pair<float,float>{sx,sy}; };

    for (auto it = q.begin(); it != q.end(); ++it) {
        auto [node, layout, style_opt, vis_opt, parent_opt, text_opt, image_opt, button_opt, scroll_opt] = *it;
        (void)node;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible)) continue;
        auto ssp = scroll_of2(static_cast<u32>(it.entity())); const float sx = ssp.first; const float sy = ssp.second;
        const int x = static_cast<int>(layout.ptr->x); const int y = static_cast<int>(layout.ptr->y); const int w = static_cast<int>(layout.ptr->w); const int h = static_cast<int>(layout.ptr->h);
        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        const int cx = x + static_cast<int>(s.padding); const int cy = y + static_cast<int>(s.padding); const int cw = w - static_cast<int>(s.padding * 2.f); const int ch = h - static_cast<int>(s.padding * 2.f);

        bool sc_apply = false; ::Rectangle sc_rect = {0,0,0,0};
        for (u32 pp = parent_opt.ptr ? static_cast<u32>(parent_opt.ptr->entity) : 0u; pp != 0u; ) {
            auto psit = styles2.find(pp); auto plit = layouts2.find(pp);
            if (psit != styles2.end() && plit != layouts2.end()) {
                const r::Style &ps = psit->second; const r::ComputedLayout *pl = plit->second;
                if (ps.clip_children || ps.overflow_clip) {
                    int scx = static_cast<int>(pl->x + ps.padding); int scy = static_cast<int>(pl->y + ps.padding);
                    int scw = static_cast<int>(pl->w - ps.padding * 2.f); int sch = static_cast<int>(pl->h - ps.padding * 2.f);
                    if (!sc_apply) { sc_rect = { static_cast<float>(scx), static_cast<float>(scy), static_cast<float>(scw), static_cast<float>(sch) }; sc_apply = true; }
                    else { sc_rect = intersect(static_cast<int>(sc_rect.x), static_cast<int>(sc_rect.y), static_cast<int>(sc_rect.width), static_cast<int>(sc_rect.height), scx, scy, scw, sch); }
                }
            }
            auto pit = parents2.find(pp); if (pit == parents2.end()) break; pp = pit->second;
        }
        if (sc_apply) BeginScissorMode(static_cast<int>(sc_rect.x), static_cast<int>(sc_rect.y), static_cast<int>(sc_rect.width), static_cast<int>(sc_rect.height));

        if (image_opt.ptr && !image_opt.ptr->path.empty()) {
            auto &cache = textures.ptr->cache; const std::string &path = image_opt.ptr->path; const ::Texture2D *tex = nullptr;
            auto itc = cache.find(path);
            if (itc == cache.end()) { auto t = LoadTexture(path.c_str()); if (t.id != 0) cache[path] = t; auto it2 = cache.find(path); if (it2 != cache.end()) tex = &it2->second; }
            else { tex = &itc->second; }
            if (tex && tex->id != 0) {
                ::Rectangle src{0,0,static_cast<float>(tex->width),static_cast<float>(tex->height)}; float dw = static_cast<float>(cw); float dh = static_cast<float>(ch); float dx = static_cast<float>(cx); float dy = static_cast<float>(cy);
                if (image_opt.ptr->keep_aspect && tex->height > 0) { float ar = static_cast<float>(tex->width) / static_cast<float>(tex->height); float box_ar = static_cast<float>(cw) / static_cast<float>(ch); if (box_ar > ar) { dw = static_cast<float>(ch) * ar; dx = static_cast<float>(cx) + (static_cast<float>(cw) - dw) * 0.5f; } else { dh = static_cast<float>(cw) / ar; dy = static_cast<float>(cy) + (static_cast<float>(ch) - dh) * 0.5f; } }
                ::Rectangle dst{dx + sx, dy + sy, dw, dh}; DrawTexturePro(*tex, src, dst, {0,0}, 0.f, { image_opt.ptr->tint.r, image_opt.ptr->tint.g, image_opt.ptr->tint.b, image_opt.ptr->tint.a });
            }
        }

        if (text_opt.ptr && !text_opt.ptr->content.empty()) {
            ::Font font = GetFontDefault();
            if (!text_opt.ptr->font_path.empty()) { const ::Font *fp = fonts.ptr->load(text_opt.ptr->font_path); if (fp) font = *fp; }
            else if (!theme.ptr->default_font_path.empty()) { const ::Font *fp = fonts.ptr->load(theme.ptr->default_font_path); if (fp) font = *fp; }
            float spacing = 1.0f; int fs = (text_opt.ptr->font_size > 0) ? text_opt.ptr->font_size : theme.ptr->default_font_size;
            ::Color col = { text_opt.ptr->color.r, text_opt.ptr->color.g, text_opt.ptr->color.b, text_opt.ptr->color.a };
            if (col.a == 0) col = { theme.ptr->text.r, theme.ptr->text.g, theme.ptr->text.b, theme.ptr->text.a };
            if (button_opt.ptr) col = { theme.ptr->button.text.r, theme.ptr->button.text.g, theme.ptr->button.text.b, theme.ptr->button.text.a };

            std::string text = text_opt.ptr->content;
            if (text_opt.ptr->wrap_width > 0.f && cw > 0) {
                float maxw = std::min(text_opt.ptr->wrap_width, static_cast<float>(cw));
                std::string out; std::string line; size_t i = 0; const size_t n = text.size();
                while (i < n) { size_t j = i; while (j < n && text[j] != ' ') j++; std::string word = text.substr(i, j - i); std::string candidate = line.empty() ? word : line + " " + word; float width = MeasureTextEx(font, candidate.c_str(), static_cast<float>(fs), spacing).x; if (width <= maxw || line.empty()) line = candidate; else { out += line + "\n"; line = word; } i = (j < n && text[j] == ' ') ? j + 1 : j; }
                if (!line.empty()) out += line;
                text = out;
            }
            auto measure_multiline = [&](const std::string &t) -> ::Vector2 {
                float maxw = 0.f; float totalh = 0.f; size_t start = 0;
                while (true) {
                    size_t nl = t.find('\n', start);
                    std::string ln = (nl == std::string::npos) ? t.substr(start) : t.substr(start, nl - start);
                    ::Vector2 sz = MeasureTextEx(font, ln.c_str(), static_cast<float>(fs), spacing);
                    if (sz.x > maxw) maxw = sz.x;
                    totalh += static_cast<float>(fs);
                    if (nl == std::string::npos) break;
                    start = nl + 1;
                }
                return ::Vector2{maxw, totalh};
            };

            ::Vector2 sz = measure_multiline(text); float tx = static_cast<float>(cx) + sx; float ty = static_cast<float>(cy) + sy;
            const bool centerX = (s.align == r::AlignItems::Center); const bool centerY = (button_opt.ptr != nullptr) || (s.justify == r::JustifyContent::Center);
            if (centerX) tx = static_cast<float>(cx) + (static_cast<float>(cw) - sz.x) * 0.5f + sx;
            if (centerY) ty = static_cast<float>(cy) + (static_cast<float>(ch) - sz.y) * 0.5f + sy;
            DrawTextEx(font, text.c_str(), { tx, ty }, static_cast<float>(fs), spacing, col);
        }
        if (sc_apply) EndScissorMode();
    }

    BeginMode3D(_to_raylib(*cam.ptr));
}

}
