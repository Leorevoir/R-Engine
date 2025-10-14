/**
 * \file UiSystems.Render.cpp
 * \brief Rendering system for the UI plugin using Raylib.
 */
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <vector>

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

/// @brief Item to be rendered with associated metadata
struct DrawItem {
        int z;                          ///< Z-index for depth sorting
        size_t order;                   ///< Creation order for stable sorting
        r::ecs::Entity handle;          ///< Entity handle
        const r::ComputedLayout *layout;///< Computed layout information
        r::Style style;                 ///< Visual style properties
        r::ecs::Entity parent;          ///< Parent entity for hierarchy
        bool is_button;                 ///< Whether this item is a button
        bool is_disabled;               ///< Whether this item is disabled
};

/// @brief Cached render data for efficient access during rendering
struct RenderData {
        std::unordered_map<r::ecs::Entity, const r::UiScroll *> scrolls;        ///< Scroll components by entity
        std::unordered_map<r::ecs::Entity, float> content_bottom;               ///< Bottom bounds of scrollable content
        std::unordered_map<r::ecs::Entity, const r::ComputedLayout *> layouts;  ///< Layout data by entity
        std::unordered_map<r::ecs::Entity, r::Style> styles;                    ///< Style data by entity
        std::unordered_map<r::ecs::Entity, r::ecs::Entity> parents;             ///< Parent entity mapping
        std::unordered_map<r::ecs::Entity, r::ecs::Entity> parent_from_children;///< Reverse parent lookup
};

/// @brief Placeholder value for invalid entity references
static constexpr auto PLACEHOLDER = std::numeric_limits<r::ecs::Entity>::max();
/// @brief Threshold value for determining placeholder entities
static constexpr auto PLACEHOLDER_THRESHOLD = std::numeric_limits<r::ecs::Entity>::max() / 2u;

/**
 * @brief Computes the intersection of two rectangles
 * @param x X coordinate of first rectangle
 * @param y Y coordinate of first rectangle  
 * @param w Width of first rectangle
 * @param h Height of first rectangle
 * @param x2 X coordinate of second rectangle
 * @param y2 Y coordinate of second rectangle
 * @param w2 Width of second rectangle
 * @param h2 Height of second rectangle
 * @return Rectangle representing the intersection area
 */
static auto intersect_rectangles(int x, int y, int w, int h, int x2, int y2, int w2, int h2) -> ::Rectangle
{
    int nx = std::max(x, x2);
    int ny = std::max(y, y2);
    int r1 = x + w;
    int r2 = x2 + w2;
    int b1 = y + h;
    int b2 = y2 + h2;
    int nw = std::max(0, std::min(r1, r2) - nx);
    int nh = std::max(0, std::min(b1, b2) - ny);
    return ::Rectangle{static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nw), static_cast<float>(nh)};
}

/**
 * @brief Resolves the actual parent entity for a given child entity
 * @param data Render data containing entity mappings
 * @param child Child entity to resolve parent for
 * @param parent Potential parent entity
 * @return Resolved parent entity or NULL_ENTITY if none
 */
static auto resolve_parent_entity(const RenderData &data, r::ecs::Entity child, r::ecs::Entity parent) -> r::ecs::Entity
{
    if (parent != PLACEHOLDER)
        return parent;
    auto it = data.parent_from_children.find(child);
    if (it != data.parent_from_children.end())
        return it->second;
    return r::ecs::NULL_ENTITY;
}

/**
 * @brief Resolves parent entity specifically for foreground rendering
 * @param data Render data containing entity mappings
 * @param child Child entity to resolve parent for
 * @param parent Potential parent entity
 * @return Resolved parent entity for foreground clipping
 */
static auto resolve_parent_for_foreground(const RenderData &data, r::ecs::Entity child, r::ecs::Entity parent) -> r::ecs::Entity
{
    if (parent < PLACEHOLDER_THRESHOLD)
        return parent;
    auto parent_it = data.parent_from_children.find(child);
    if (parent_it != data.parent_from_children.end())
        return parent_it->second;
    return r::ecs::NULL_ENTITY;
}

/**
 * @brief Calculates the scroll offset for a given entity
 * @param data Render data containing scroll information
 * @param entity Entity to calculate scroll offset for
 * @return Pair of (x_offset, y_offset) for scrolling
 */
static auto calculate_scroll_offset(const RenderData &data, r::ecs::Entity entity) -> std::pair<float, float>
{
    float sx = 0.f, sy = 0.f;
    auto p = entity;
    while (p != r::ecs::NULL_ENTITY) {
        auto pit = data.parents.find(p);
        if (pit == data.parents.end())
            break;
        p = resolve_parent_entity(data, p, pit->second);
        auto sit = data.styles.find(p);
        auto scit = data.scrolls.find(p);
        if (sit != data.styles.end() && (sit->second.clip_children || sit->second.overflow_clip) && scit != data.scrolls.end()) {
            sx -= scit->second->x;
            sy -= scit->second->y;
        }
    }
    return std::make_pair(sx, sy);
}

static auto calculate_scroll_offset_for_foreground(const RenderData &data, r::ecs::Entity entity) -> std::pair<float, float>
{
    float sx = 0.f, sy = 0.f;
    auto p = entity;
    while (p != r::ecs::NULL_ENTITY) {
        auto pit = data.parents.find(p);
        if (pit == data.parents.end())
            break;
        p = pit->second;
        auto sit = data.styles.find(p);
        auto scit = data.scrolls.find(p);
        if (sit != data.styles.end() && (sit->second.clip_children || sit->second.overflow_clip) && scit != data.scrolls.end()) {
            sx -= scit->second->x;
            sy -= scit->second->y;
        }
    }
    return std::make_pair(sx, sy);
}

/**
 * @brief Collects and sorts UI elements for rendering
 * @tparam QueryType Type of the ECS query
 * @param q ECS query containing UI entities
 * @return Pair of sorted draw items and associated render data
 */
template<typename QueryType>
static auto collect_draw_items(QueryType &q) -> std::pair<std::vector<DrawItem>, RenderData>
{
    std::vector<DrawItem> items;
    items.reserve(128);
    RenderData data;

    size_t ord = 0;
    for (auto it = q.begin(); it != q.end(); ++it) {
        auto [node, layout, style_opt, vis_opt, parent_opt, text_opt, image_opt, button_opt, scroll_opt, children_opt] = *it;
        (void) node;
        (void) text_opt;
        (void) image_opt;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible))
            continue;

        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        const bool is_button = button_opt.ptr != nullptr;
        const bool is_disabled = (button_opt.ptr && button_opt.ptr->disabled);
        const auto id = static_cast<r::ecs::Entity>(it.entity());
        const auto pid = parent_opt.ptr ? parent_opt.ptr->entity : r::ecs::NULL_ENTITY;

        items.push_back({s.z_index, ord++, id, layout.ptr, s, pid, is_button, is_disabled});
        data.layouts[id] = layout.ptr;
        data.styles[id] = s;
        data.parents[id] = pid;

        if (scroll_opt.ptr)
            data.scrolls[id] = scroll_opt.ptr;
        if (children_opt.ptr) {
            for (auto child : children_opt.ptr->entities) {
                data.parent_from_children[child] = id;
            }
        }
    }

    std::stable_sort(items.begin(), items.end(), [](const DrawItem &a, const DrawItem &b) {
        if (a.z != b.z)
            return a.z < b.z;
        return a.order < b.order;
    });

    return std::make_pair(std::move(items), std::move(data));
}

/**
 * @brief Sets up parent-child entity mappings for rendering
 * @param data Render data to configure mappings for
 */
static void setup_entity_mappings(RenderData &data)
{
    for (auto &kv : data.parents) {
        auto it = data.parent_from_children.find(kv.first);
        if (it != data.parent_from_children.end()) {
            kv.second = it->second;
        } else if (kv.second == PLACEHOLDER) {
            kv.second = r::ecs::NULL_ENTITY;
        }
    }
}

/**
 * @brief Calculates the bottom bounds of scrollable content areas
 * @param items Vector of UI items to analyze
 * @param data Render data to store content bottom values
 */
static void calculate_content_bottoms(const std::vector<DrawItem> &items, RenderData &data)
{
    for (const auto &it : items) {
        auto p_it = data.parents.find(it.handle);
        if (p_it != data.parents.end()) {
            auto parent_id = resolve_parent_entity(data, it.handle, p_it->second);
            if (data.scrolls.find(parent_id) != data.scrolls.end()) {
                float bottom = it.layout->y + it.layout->h;
                data.content_bottom[parent_id] = std::max(data.content_bottom[parent_id], bottom);
            }
        }
    }
}

static void apply_scissor_mode(const DrawItem &item, const RenderData &data, bool &applied_scissor, ::Rectangle &scissor)
{
    applied_scissor = false;
    scissor = {0, 0, 0, 0};

    auto p = item.handle;
    bool first = true;
    while (p != r::ecs::NULL_ENTITY) {
        auto psit = data.styles.find(p);
        auto plit = data.layouts.find(p);
        if (psit != data.styles.end() && plit != data.layouts.end()) {
            const r::Style &ps = psit->second;
            const r::ComputedLayout *pl = plit->second;
            if (ps.clip_children || ps.overflow_clip) {
                const int cx = static_cast<int>(pl->x + ps.padding);
                const int cy = static_cast<int>(pl->y + ps.padding);
                const int cw = static_cast<int>(pl->w - ps.padding * 2.f);
                const int ch = static_cast<int>(pl->h - ps.padding * 2.f);
                if (first) {
                    scissor = {static_cast<float>(cx), static_cast<float>(cy), static_cast<float>(cw), static_cast<float>(ch)};
                    first = false;
                } else {
                    scissor = intersect_rectangles(static_cast<int>(scissor.x), static_cast<int>(scissor.y),
                        static_cast<int>(scissor.width), static_cast<int>(scissor.height), cx, cy, cw, ch);
                }
                applied_scissor = true;
            }
        }
        auto pit = data.parents.find(p);
        if (pit == data.parents.end())
            break;
        p = pit->second;
    }
}

/**
 * @brief Renders background rectangles and borders for UI elements
 * @param items Vector of UI items to render backgrounds for
 * @param data Render data containing layout and style information
 * @param input UI input state for button styling
 * @param theme UI theme for default colors and styling
 */
static void render_backgrounds(const std::vector<DrawItem> &items, const RenderData &data, r::ecs::Res<r::UiInputState> input,
    r::ecs::Res<r::UiTheme> theme)
{
    for (const auto &it : items) {
        auto ss = calculate_scroll_offset(data, it.handle);
        const float sx = ss.first;
        const float sy = ss.second;
        const int x = static_cast<int>(it.layout->x);
        const int y = static_cast<int>(it.layout->y);
        const int w = static_cast<int>(it.layout->w);
        const int h = static_cast<int>(it.layout->h);

        bool applied_scissor = false;
        ::Rectangle scissor = {0, 0, 0, 0};
        apply_scissor_mode(it, data, applied_scissor, scissor);

        if (applied_scissor) {
            BeginScissorMode(static_cast<int>(scissor.x), static_cast<int>(scissor.y), static_cast<int>(scissor.width),
                static_cast<int>(scissor.height));
        }

        r::Color bg = it.style.background;
        r::Color border = it.style.border_color;
        float border_thickness = it.style.border_thickness;

        if (it.is_button) {
            if (it.is_disabled) {
                bg = theme.ptr->button.bg_disabled;
                border = theme.ptr->button.border_disabled;
                border_thickness = theme.ptr->button.border_thickness;
            } else if (input.ptr->active == it.handle) {
                bg = theme.ptr->button.bg_pressed;
                border = theme.ptr->button.border_pressed;
                border_thickness = theme.ptr->button.border_thickness;
            } else if (input.ptr->hovered == it.handle) {
                bg = theme.ptr->button.bg_hover;
                border = theme.ptr->button.border_hover;
                border_thickness = theme.ptr->button.border_thickness;
            } else {
                bg = theme.ptr->button.bg_normal;
                border = theme.ptr->button.border_normal;
                border_thickness = theme.ptr->button.border_thickness;
            }
        }
        if (!it.is_button && bg.a == 0) {
            bg = theme.ptr->panel_bg;
        }

        DrawRectangle(static_cast<int>(static_cast<float>(x) + sx), static_cast<int>(static_cast<float>(y) + sy), w, h,
            {bg.r, bg.g, bg.b, bg.a});

        if (border_thickness > 0.f) {
            ::Rectangle rec{static_cast<float>(x) + sx, static_cast<float>(y) + sy, static_cast<float>(w), static_cast<float>(h)};
            DrawRectangleLinesEx(rec, static_cast<float>(border_thickness), {border.r, border.g, border.b, border.a});
        }

        if (applied_scissor)
            EndScissorMode();
    }
}

/**
 * @brief Renders debug overlay and UI element bounds visualization
 * @tparam QueryType Type of the ECS query
 * @param cfg Plugin configuration for debug settings
 * @param q ECS query containing UI entities
 * @param input UI input state for highlighting active elements
 */
template<typename QueryType>
static void render_debug_overlay(r::ecs::Res<UiPluginConfig> cfg, QueryType &q, r::ecs::Res<r::UiInputState> input)
{
    if (cfg.ptr->show_debug_overlay) {
        DrawRectangle(8, 8, 220, 28, {255, 255, 255, 200});
        const char *text = cfg.ptr->overlay_text.c_str();
        const int font_size = 18;
        DrawText(text, 14, 12, font_size, {0, 0, 0, 255});
        DrawFPS(10, 40);
    }

    if (cfg.ptr->debug_draw_bounds) {
        for (auto it = q.begin(); it != q.end(); ++it) {
            auto [node, layout, style_opt, vis_opt, parent_opt, text_opt, image_opt, button_opt, scroll_opt, children_opt] = *it;
            (void) node;
            (void) style_opt;
            (void) vis_opt;
            (void) text_opt;
            (void) image_opt;
            (void) button_opt;
            (void) scroll_opt;
            (void) parent_opt;
            (void) children_opt;
            const int x = static_cast<int>(layout.ptr->x);
            const int y = static_cast<int>(layout.ptr->y);
            const int w = static_cast<int>(layout.ptr->w);
            const int h = static_cast<int>(layout.ptr->h);
            DrawRectangleLines(x, y, w, h, {120, 120, 120, 120});
        }

        auto draw_highlight = [&](r::ecs::Entity h, ::Color c) {
            if (h == r::ecs::NULL_ENTITY)
                return;
            for (auto it = q.begin(); it != q.end(); ++it) {
                auto [node, layout, sopt, vopt, popt, topt, iopt, bopt, scopt, chopt] = *it;
                (void) node;
                (void) sopt;
                (void) vopt;
                (void) popt;
                (void) topt;
                (void) iopt;
                (void) bopt;
                (void) scopt;
                (void) chopt;
                auto eid = static_cast<r::ecs::Entity>(it.entity());
                if (eid == h) {
                    ::Rectangle r{layout.ptr->x, layout.ptr->y, layout.ptr->w, layout.ptr->h};
                    DrawRectangleLinesEx(r, 2, c);
                    break;
                }
            }
        };
        draw_highlight(input.ptr->hovered, {0, 255, 0, 200});
        draw_highlight(input.ptr->active, {255, 165, 0, 200});
        draw_highlight(input.ptr->focused, {255, 255, 0, 200});
    }
}

/**
 * @brief Renders scrollbars for scrollable UI containers
 * @param data Render data containing scroll container information
 * @param theme UI theme for scrollbar styling
 */
static void render_scrollbars(const RenderData &data, r::ecs::Res<r::UiTheme> theme)
{
    for (const auto &kv : data.scrolls) {
        auto cont = kv.first;
        const auto *pl = data.layouts.at(cont);
        const r::Style ps = data.styles.at(cont);
        const float pad = (ps.padding > 0.f) ? ps.padding : static_cast<float>(theme.ptr->padding);
        const float viewport_h = pl->h - pad * 2.f;
        const float content_h = std::max(0.f, data.content_bottom.at(cont) - (pl->y + pad));

        if (content_h <= viewport_h + 1.f)
            continue;

        const float max_scroll = content_h - viewport_h;
        const float scroll_y = std::min(std::max(0.f, kv.second->y), max_scroll);
        const int track_x = static_cast<int>(pl->x + pl->w - 6);
        const int track_y = static_cast<int>(pl->y + pad);
        const int track_w = 4;
        const int track_h = static_cast<int>(viewport_h);

        DrawRectangle(track_x, track_y, track_w, track_h, {70, 70, 70, 180});

        const int thumb_h = std::max(20, static_cast<int>(viewport_h * (viewport_h / content_h)));
        const int thumb_y = track_y + static_cast<int>((viewport_h - static_cast<float>(thumb_h)) * (scroll_y / max_scroll));

        DrawRectangle(track_x, thumb_y, track_w, thumb_h, {200, 200, 200, 220});
        DrawRectangleLines(track_x, track_y, track_w, track_h, {220, 220, 220, 120});
    }
}

static void apply_foreground_scissor_mode(r::ecs::Entity current_entity, r::ecs::Optional<r::ecs::Parent> parent_opt,
    const RenderData &data, bool &sc_apply, ::Rectangle &sc_rect)
{
    sc_apply = false;
    sc_rect = {0, 0, 0, 0};

    auto pp = parent_opt.ptr ? resolve_parent_for_foreground(data, current_entity, parent_opt.ptr->entity) : r::ecs::NULL_ENTITY;
    bool first = true;
    while (pp != r::ecs::NULL_ENTITY) {
        auto psit = data.styles.find(pp);
        auto plit = data.layouts.find(pp);
        if (psit != data.styles.end() && plit != data.layouts.end()) {
            const r::Style &ps = psit->second;
            const r::ComputedLayout *pl = plit->second;
            if (ps.clip_children || ps.overflow_clip) {
                int scx = static_cast<int>(pl->x + ps.padding);
                int scy = static_cast<int>(pl->y + ps.padding);
                int scw = static_cast<int>(pl->w - ps.padding * 2.f);
                int sch = static_cast<int>(pl->h - ps.padding * 2.f);
                if (first) {
                    sc_rect = {static_cast<float>(scx), static_cast<float>(scy), static_cast<float>(scw), static_cast<float>(sch)};
                    first = false;
                } else {
                    sc_rect = intersect_rectangles(static_cast<int>(sc_rect.x), static_cast<int>(sc_rect.y),
                        static_cast<int>(sc_rect.width), static_cast<int>(sc_rect.height), scx, scy, scw, sch);
                }
                sc_apply = true;
            }
        }
        auto pit = data.parents.find(pp);
        if (pit == data.parents.end())
            break;
        pp = resolve_parent_for_foreground(data, pp, pit->second);
    }
}

static void render_image(r::ecs::Optional<r::UiImage> image_opt, int cx, int cy, int cw, int ch, float sx, float sy,
    r::ecs::ResMut<r::UiTextures> textures)
{
    if (!image_opt.ptr || image_opt.ptr->path.empty())
        return;

    auto &cache = textures.ptr->cache;
    const std::string &path = image_opt.ptr->path;
    const ::Texture2D *tex = nullptr;

    auto itc = cache.find(path);
    if (itc == cache.end()) {
        auto t = LoadTexture(path.c_str());
        if (t.id != 0)
            cache[path] = t;
        auto it2 = cache.find(path);
        if (it2 != cache.end())
            tex = &it2->second;
    } else {
        tex = &itc->second;
    }

    if (tex && tex->id != 0) {
        ::Rectangle src{0, 0, static_cast<float>(tex->width), static_cast<float>(tex->height)};
        float dw = static_cast<float>(cw);
        float dh = static_cast<float>(ch);
        float dx = static_cast<float>(cx);
        float dy = static_cast<float>(cy);

        if (image_opt.ptr->keep_aspect && tex->height > 0) {
            float ar = static_cast<float>(tex->width) / static_cast<float>(tex->height);
            float box_ar = static_cast<float>(cw) / static_cast<float>(ch);
            if (box_ar > ar) {
                dw = static_cast<float>(ch) * ar;
                dx = static_cast<float>(cx) + (static_cast<float>(cw) - dw) * 0.5f;
            } else {
                dh = static_cast<float>(cw) / ar;
                dy = static_cast<float>(cy) + (static_cast<float>(ch) - dh) * 0.5f;
            }
        }

        ::Rectangle dst{dx + sx, dy + sy, dw, dh};
        DrawTexturePro(*tex, src, dst, {0, 0}, 0.f,
            {image_opt.ptr->tint.r, image_opt.ptr->tint.g, image_opt.ptr->tint.b, image_opt.ptr->tint.a});
    }
}

static auto measure_multiline_text(const std::string &text, ::Font font, int font_size, float spacing) -> ::Vector2
{
    float maxw = 0.f;
    float totalh = 0.f;
    size_t start = 0;
    while (true) {
        size_t nl = text.find('\n', start);
        std::string ln = (nl == std::string::npos) ? text.substr(start) : text.substr(start, nl - start);
        ::Vector2 sz = MeasureTextEx(font, ln.c_str(), static_cast<float>(font_size), spacing);
        if (sz.x > maxw)
            maxw = sz.x;
        totalh += static_cast<float>(font_size);
        if (nl == std::string::npos)
            break;
        start = nl + 1;
    }
    return ::Vector2{maxw, totalh};
}

static auto wrap_text(const std::string &text, ::Font font, int font_size, float spacing, float wrap_width, int content_width)
    -> std::string
{
    if (wrap_width <= 0.f || content_width <= 0)
        return text;

    float maxw = std::min(wrap_width, static_cast<float>(content_width));
    std::string out;
    std::string line;
    size_t i = 0;
    const size_t n = text.size();

    while (i < n) {
        size_t j = i;
        while (j < n && text[j] != ' ')
            j++;
        std::string word = text.substr(i, j - i);
        std::string candidate = line.empty() ? word : line + " " + word;
        float width = MeasureTextEx(font, candidate.c_str(), static_cast<float>(font_size), spacing).x;

        if (width <= maxw || line.empty()) {
            line = candidate;
        } else {
            out += line + "\n";
            line = word;
        }
        i = (j < n && text[j] == ' ') ? j + 1 : j;
    }
    if (!line.empty())
        out += line;
    return out;
}

static void render_text(r::ecs::Optional<r::UiText> text_opt, r::ecs::Optional<r::UiButton> button_opt, const r::Style &s, int cx, int cy,
    int cw, int ch, float sx, float sy, r::ecs::ResMut<r::UiFonts> fonts, r::ecs::Res<r::UiTheme> theme)
{
    if (!text_opt.ptr || text_opt.ptr->content.empty())
        return;

    ::Font font = GetFontDefault();
    if (!text_opt.ptr->font_path.empty()) {
        const ::Font *fp = fonts.ptr->load(text_opt.ptr->font_path);
        if (fp)
            font = *fp;
    } else if (!theme.ptr->default_font_path.empty()) {
        const ::Font *fp = fonts.ptr->load(theme.ptr->default_font_path);
        if (fp)
            font = *fp;
    }

    float spacing = 1.0f;
    int fs = (text_opt.ptr->font_size > 0) ? text_opt.ptr->font_size : theme.ptr->default_font_size;
    ::Color col = {text_opt.ptr->color.r, text_opt.ptr->color.g, text_opt.ptr->color.b, text_opt.ptr->color.a};

    if (col.a == 0)
        col = {theme.ptr->text.r, theme.ptr->text.g, theme.ptr->text.b, theme.ptr->text.a};
    if (button_opt.ptr)
        col = {theme.ptr->button.text.r, theme.ptr->button.text.g, theme.ptr->button.text.b, theme.ptr->button.text.a};

    std::string text = wrap_text(text_opt.ptr->content, font, fs, spacing, text_opt.ptr->wrap_width, cw);
    ::Vector2 sz = measure_multiline_text(text, font, fs, spacing);

    float tx = static_cast<float>(cx) + sx;
    float ty = static_cast<float>(cy) + sy;

    const bool centerX = (s.align == r::AlignItems::Center);
    const bool centerY = (button_opt.ptr != nullptr) || (s.justify == r::JustifyContent::Center);

    if (centerX)
        tx = static_cast<float>(cx) + (static_cast<float>(cw) - sz.x) * 0.5f + sx;
    if (centerY)
        ty = static_cast<float>(cy) + (static_cast<float>(ch) - sz.y) * 0.5f + sy;

    DrawTextEx(font, text.c_str(), {tx, ty}, static_cast<float>(fs), spacing, col);
}

/**
 * @brief Renders foreground content (text and images) for UI elements
 * @tparam QueryType Type of the ECS query
 * @param q ECS query containing UI entities
 * @param data Render data for clipping and positioning
 * @param textures Texture cache for image rendering
 * @param fonts Font cache for text rendering
 * @param theme UI theme for default styling
 */
template<typename QueryType>
static void render_foreground(QueryType &q, const RenderData &data, r::ecs::ResMut<r::UiTextures> textures,
    r::ecs::ResMut<r::UiFonts> fonts, r::ecs::Res<r::UiTheme> theme)
{
    for (auto it = q.begin(); it != q.end(); ++it) {
        auto [node, layout, style_opt, vis_opt, parent_opt, text_opt, image_opt, button_opt, scroll_opt, children_opt] = *it;
        (void) node;
        if (vis_opt.ptr && (*vis_opt.ptr != r::Visibility::Visible))
            continue;

        auto ssp = calculate_scroll_offset_for_foreground(data, static_cast<r::ecs::Entity>(it.entity()));
        const float sx = ssp.first;
        const float sy = ssp.second;
        const int x = static_cast<int>(layout.ptr->x);
        const int y = static_cast<int>(layout.ptr->y);
        const int w = static_cast<int>(layout.ptr->w);
        const int h = static_cast<int>(layout.ptr->h);
        const r::Style s = style_opt.ptr ? *style_opt.ptr : r::Style{};
        const int cx = x + static_cast<int>(s.padding);
        const int cy = y + static_cast<int>(s.padding);
        const int cw = w - static_cast<int>(s.padding * 2.f);
        const int ch = h - static_cast<int>(s.padding * 2.f);

        bool sc_apply = false;
        ::Rectangle sc_rect = {0, 0, 0, 0};
        apply_foreground_scissor_mode(static_cast<r::ecs::Entity>(it.entity()), parent_opt, data, sc_apply, sc_rect);

        if (sc_apply) {
            BeginScissorMode(static_cast<int>(sc_rect.x), static_cast<int>(sc_rect.y), static_cast<int>(sc_rect.width),
                static_cast<int>(sc_rect.height));
        }

        render_image(image_opt, cx, cy, cw, ch, sx, sy, textures);
        render_text(text_opt, button_opt, s, cx, cy, cw, ch, sx, sy, fonts, theme);

        if (sc_apply)
            EndScissorMode();
    }
}

void render_system(r::ecs::Res<UiPluginConfig> cfg, r::ecs::Res<r::Camera3d> cam, r::ecs::Res<r::UiInputState> input,
    r::ecs::Res<r::UiTheme> theme, r::ecs::ResMut<r::UiTextures> textures, r::ecs::ResMut<r::UiFonts> fonts,
    r::ecs::Query<r::ecs::Ref<r::UiNode>, r::ecs::Ref<r::ComputedLayout>, r::ecs::Optional<r::Style>, r::ecs::Optional<r::Visibility>,
        r::ecs::Optional<r::ecs::Parent>, r::ecs::Optional<r::UiText>, r::ecs::Optional<r::UiImage>, r::ecs::Optional<r::UiButton>,
        r::ecs::Optional<r::UiScroll>, r::ecs::Optional<r::ecs::Children>>
        q) noexcept
{
    (void) cam;

    auto [items, data] = collect_draw_items(q);
    setup_entity_mappings(data);
    calculate_content_bottoms(items, data);
    render_backgrounds(items, data, input, theme);
    render_debug_overlay(cfg, q, input);
    render_scrollbars(data, theme);
    render_foreground(q, data, textures, fonts, theme);
}

}// namespace r::ui
