#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <unordered_map>
#include <vector>
#include <algorithm>

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
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>, r::ecs::Optional<r::Parent>, r::ecs::Optional<r::UiText>, r::ecs::Optional<r::UiImage>, r::ecs::Optional<r::UiButton>, r::ecs::Optional<r::UiScroll>, r::ecs::EntityId> q) noexcept
{
    EndMode3D();

    struct DrawItem {
        int z; size_t order; r::ecs::Entity id; const r::ComputedLayout *layout; r::Style style; r::ecs::Entity parent; bool is_button; bool is_disabled;
    };

    std::vector<DrawItem> items; items.reserve(128);
    std::unordered_map<r::ecs::Entity, const r::UiScroll*> scrolls;
    std::unordered_map<r::ecs::Entity, float> content_bottom;
    std::unordered_map<r::ecs::Entity, const r::ComputedLayout *> layouts;
    std::unordered_map<r::ecs::Entity, r::Style> styles;
    std::unordered_map<r::ecs::Entity, r::ecs::Entity> parents;

    size_t ord = 0;
    for (auto [node, layout, style_opt, vis_opt, parent_opt, text_opt, image_opt, button_opt, scroll_opt, id] : q) {
        (void)node; (void)text_opt; (void)image_opt;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible)) continue;
        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        const bool is_button = button_opt.ptr != nullptr;
        const bool is_disabled = (button_opt.ptr && button_opt.ptr->disabled);
        items.push_back({ s.z_index, ord++, id.value, layout.ptr, s, parent_opt.ptr ? parent_opt.ptr->id : 0, is_button, is_disabled });
        layouts[id.value] = layout.ptr; styles[id.value] = s; parents[id.value] = parent_opt.ptr ? parent_opt.ptr->id : 0;
        if (scroll_opt.ptr) scrolls[id.value] = scroll_opt.ptr;
    }

    std::stable_sort(items.begin(), items.end(), [](const DrawItem &a, const DrawItem &b) { if (a.z != b.z) return a.z < b.z; return a.order < b.order; });

    auto intersect = [](int x, int y, int w, int h, int x2, int y2, int w2, int h2) {
        int nx = std::max(x, x2); int ny = std::max(y, y2); int r1 = x + w; int r2 = x2 + w2; int b1 = y + h; int b2 = y2 + h2;
        int nw = std::max(0, std::min(r1, r2) - nx); int nh = std::max(0, std::min(b1, b2) - ny);
        return ::Rectangle{ (float)nx, (float)ny, (float)nw, (float)nh };
    };

    auto scroll_of = [&](r::ecs::Entity e){ float sx=0.f, sy=0.f; r::ecs::Entity p = e; while (p!=0){ auto pit=parents.find(p); if (pit==parents.end()) break; p = pit->second; auto sit=styles.find(p); auto scit = scrolls.find(p); if (sit!=styles.end() && (sit->second.clip_children || sit->second.overflow_clip) && scit!=scrolls.end()) { sx -= scit->second->x; sy -= scit->second->y; } } return std::pair<float,float>{sx,sy}; };

    for (const auto &it : items) {
        auto p_it = parents.find(it.id);
        if (p_it != parents.end()) {
            auto parent_id = p_it->second;
            if (scrolls.find(parent_id) != scrolls.end()) {
                float bottom = it.layout->y + it.layout->h;
                content_bottom[parent_id] = std::max(content_bottom[parent_id], bottom);
            }
        }
    }

    for (const auto &it : items) {
        auto ss = scroll_of(it.id);
        const float sx = ss.first; const float sy = ss.second;
        const int x = (int)it.layout->x; const int y = (int)it.layout->y; const int w = (int)it.layout->w; const int h = (int)it.layout->h;

        bool applied_scissor = false; ::Rectangle scissor = {0, 0, 0, 0};
        r::ecs::Entity p = it.parent; bool first = true;
        while (p != 0) {
            auto psit = styles.find(p); auto plit = layouts.find(p);
            if (psit != styles.end() && plit != layouts.end()) {
                const r::Style &ps = psit->second; const r::ComputedLayout *pl = plit->second;
                if (ps.clip_children) {
                    const int cx = (int)(pl->x + ps.padding); const int cy = (int)(pl->y + ps.padding);
                    const int cw = (int)(pl->w - ps.padding * 2.f); const int ch = (int)(pl->h - ps.padding * 2.f);
                    if (first) { scissor = { (float)cx, (float)cy, (float)cw, (float)ch }; first = false; }
                    else { scissor = intersect((int)scissor.x, (int)scissor.y, (int)scissor.width, (int)scissor.height, cx, cy, cw, ch); }
                    applied_scissor = true;
                }
            }
            auto pit = parents.find(p); if (pit == parents.end()) break; p = pit->second;
        }

        if (applied_scissor) BeginScissorMode((int)scissor.x, (int)scissor.y, (int)scissor.width, (int)scissor.height);

        r::Color bg = it.style.background; r::Color border = it.style.border_color; float border_thickness = it.style.border_thickness;

        if (it.is_button) {
            if (it.is_disabled) { bg = theme.ptr->button.bg_disabled; border = theme.ptr->button.border_disabled; border_thickness = theme.ptr->button.border_thickness; }
            else if (input.ptr->active == it.id) { bg = theme.ptr->button.bg_pressed; border = theme.ptr->button.border_pressed; border_thickness = theme.ptr->button.border_thickness; }
            else if (input.ptr->hovered == it.id) { bg = theme.ptr->button.bg_hover; border = theme.ptr->button.border_hover; border_thickness = theme.ptr->button.border_thickness; }
            else { bg = theme.ptr->button.bg_normal; border = theme.ptr->button.border_normal; border_thickness = theme.ptr->button.border_thickness; }
        }
        if (!it.is_button && bg.a == 0) { bg = theme.ptr->panel_bg; }

        DrawRectangle((int)((float)x + sx), (int)((float)y + sy), w, h, {bg.r, bg.g, bg.b, bg.a});
        if (border_thickness > 0.f) {
            ::Rectangle rec{ ((float)x + sx), ((float)y + sy), (float)w, (float)h };
            DrawRectangleLinesEx(rec, (int)border_thickness, { border.r, border.g, border.b, border.a });
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
        for (auto [node, layout, style_opt, vis_opt, parent_opt, text_opt, image_opt, button_opt, scroll_opt, id] : q) {
            (void)node; (void)style_opt; (void)vis_opt; (void)parent_opt; (void)text_opt; (void)image_opt; (void)button_opt; (void)scroll_opt;
            const int x = (int)layout.ptr->x; const int y = (int)layout.ptr->y; const int w = (int)layout.ptr->w; const int h = (int)layout.ptr->h;
            DrawRectangleLines(x, y, w, h, {120,120,120,120});
        }
        auto draw_highlight = [&](r::ecs::Entity e, ::Color c){ if (e==0) return; for (auto [node, layout, sopt, vopt, popt, topt, iopt, bopt, scopt, id] : q){ if (id.value==e){ DrawRectangleLinesEx({layout.ptr->x, layout.ptr->y, layout.ptr->w, layout.ptr->h}, 2, c); break; } } };
        draw_highlight(input.ptr->hovered, {0,255,0,200});
        draw_highlight(input.ptr->active, {255,165,0,200});
        draw_highlight(input.ptr->focused, {255,255,0,200});
    }

    /* Scrollbars */
    for (const auto &kv : scrolls) {
        r::ecs::Entity cont = kv.first; const auto *pl = layouts[cont]; const r::Style ps = styles[cont];
        const float pad = (ps.padding > 0.f) ? ps.padding : (float)theme.ptr->padding;
        const float viewport_h = pl->h - pad * 2.f;
        const float content_h = std::max(0.f, content_bottom[cont] - (pl->y + pad));
        if (content_h <= viewport_h + 1.f) continue;
        const float max_scroll = content_h - viewport_h; const float scroll_y = std::min(std::max(0.f, kv.second->y), max_scroll);
        const int track_x = (int)(pl->x + pl->w - 6); const int track_y = (int)(pl->y + pad); const int track_w = 4; const int track_h = (int)viewport_h;
        DrawRectangle(track_x, track_y, track_w, track_h, {70,70,70,180});
        const int thumb_h = std::max(20, (int)(viewport_h * (viewport_h / content_h)));
        const int thumb_y = track_y + (int)(((float)viewport_h - (float)thumb_h) * (scroll_y / max_scroll));
        DrawRectangle(track_x, thumb_y, track_w, thumb_h, {200,200,200,220});
        DrawRectangleLines(track_x, track_y, track_w, track_h, {220,220,220,120});
    }

    /* Foreground: images and text */
    std::unordered_map<r::ecs::Entity, const r::ComputedLayout *> layouts2; layouts2 = layouts; // alias for reuse in loop below
    std::unordered_map<r::ecs::Entity, r::Style> styles2; styles2 = styles;
    std::unordered_map<r::ecs::Entity, r::ecs::Entity> parents2; parents2 = parents;

    auto scroll_of2 = [&](r::ecs::Entity e){ float sx=0.f, sy=0.f; r::ecs::Entity p = e; while (p!=0){ auto pit=parents2.find(p); if (pit==parents2.end()) break; p = pit->second; auto sit=styles2.find(p); auto scit = scrolls.find(p); if (sit!=styles2.end() && (sit->second.clip_children || sit->second.overflow_clip) && scit!=scrolls.end()) { sx -= scit->second->x; sy -= scit->second->y; } } return std::pair<float,float>{sx,sy}; };

    for (auto [node, layout, style_opt, vis_opt, parent_opt, text_opt, image_opt, button_opt, scroll_opt, id] : q) {
        (void)node;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible)) continue;
        auto ssp = scroll_of2(id.value); const float sx = ssp.first; const float sy = ssp.second;
        const int x = (int)layout.ptr->x; const int y = (int)layout.ptr->y; const int w = (int)layout.ptr->w; const int h = (int)layout.ptr->h;
        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        const int cx = x + (int)s.padding; const int cy = y + (int)s.padding; const int cw = w - (int)(s.padding * 2.f); const int ch = h - (int)(s.padding * 2.f);

        bool sc_apply = false; ::Rectangle sc_rect = {0,0,0,0};
        for (r::ecs::Entity pp = parent_opt.ptr ? parent_opt.ptr->id : 0; pp != 0; ) {
            auto psit = styles2.find(pp); auto plit = layouts2.find(pp);
            if (psit != styles2.end() && plit != layouts2.end()) {
                const r::Style &ps = psit->second; const r::ComputedLayout *pl = plit->second;
                if (ps.clip_children || ps.overflow_clip) {
                    int scx = (int)(pl->x + ps.padding); int scy = (int)(pl->y + ps.padding);
                    int scw = (int)(pl->w - ps.padding * 2.f); int sch = (int)(pl->h - ps.padding * 2.f);
                    if (!sc_apply) { sc_rect = { (float)scx, (float)scy, (float)scw, (float)sch }; sc_apply = true; }
                    else { sc_rect = intersect((int)sc_rect.x, (int)sc_rect.y, (int)sc_rect.width, (int)sc_rect.height, scx, scy, scw, sch); }
                }
            }
            auto pit = parents2.find(pp); if (pit == parents2.end()) break; pp = pit->second;
        }
        if (sc_apply) BeginScissorMode((int)sc_rect.x, (int)sc_rect.y, (int)sc_rect.width, (int)sc_rect.height);

        if (image_opt.ptr && !image_opt.ptr->path.empty()) {
            auto &cache = textures.ptr->cache; const std::string &path = image_opt.ptr->path; const ::Texture2D *tex = nullptr;
            auto it = cache.find(path);
            if (it == cache.end()) { auto t = LoadTexture(path.c_str()); if (t.id != 0) cache[path] = t; auto it2 = cache.find(path); if (it2 != cache.end()) tex = &it2->second; }
            else { tex = &it->second; }
            if (tex && tex->id != 0) {
                ::Rectangle src{0,0,(float)tex->width,(float)tex->height}; float dw = (float)cw; float dh = (float)ch; float dx = (float)cx; float dy = (float)cy;
                if (image_opt.ptr->keep_aspect && tex->height > 0) { float ar = (float)tex->width / (float)tex->height; float box_ar = (float)cw / (float)ch; if (box_ar > ar) { dw = (float)ch * ar; dx = (float)cx + ((float)cw - dw) * 0.5f; } else { dh = (float)cw / ar; dy = (float)cy + ((float)ch - dh) * 0.5f; } }
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
                float maxw = std::min(text_opt.ptr->wrap_width, (float)cw);
                std::string out; std::string line; size_t i = 0; const size_t n = text.size();
                while (i < n) { size_t j = i; while (j < n && text[j] != ' ') j++; std::string word = text.substr(i, j - i); std::string candidate = line.empty() ? word : line + " " + word; float width = MeasureTextEx(font, candidate.c_str(), (float)fs, spacing).x; if (width <= maxw || line.empty()) line = candidate; else { out += line + "\n"; line = word; } i = (j < n && text[j] == ' ') ? j + 1 : j; }
                if (!line.empty()) out += line;
                text = out;
            }
            auto measure_multiline = [&](const std::string &t) -> ::Vector2 { float maxw = 0.f; float totalh = 0.f; size_t start = 0; while (true) { size_t end = t.find('\n', start); std::string ln = (end == std::string::npos) ? t.substr(start) : t.substr(start, end - start); ::Vector2 sz = MeasureTextEx(font, ln.c_str(), (float)fs, spacing); if (sz.x > maxw) maxw = sz.x; totalh += (float)fs; if (end == std::string::npos) break; start = end + 1; } return ::Vector2{maxw, totalh}; };

            ::Vector2 sz = measure_multiline(text); float tx = (float)cx + sx; float ty = (float)cy + sy;
            const bool centerX = (s.align == r::AlignItems::Center); const bool centerY = (button_opt.ptr != nullptr) || (s.justify == r::JustifyContent::Center);
            if (centerX) tx = (float)cx + ((float)cw - sz.x) * 0.5f + sx;
            if (centerY) ty = (float)cy + ((float)ch - sz.y) * 0.5f + sy;
            DrawTextEx(font, text.c_str(), { tx, ty }, (float)fs, spacing, col);
        }
        if (sc_apply) EndScissorMode();
    }

    BeginMode3D(_to_raylib(*cam.ptr));
}

} // namespace r::ui
