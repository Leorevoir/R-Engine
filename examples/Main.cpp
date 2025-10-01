#include <R-Engine/Application.hpp>
#include <R-Engine/Types.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/Plugins/UIPlugin.hpp>

#include <raylib.h>

#include <algorithm>
#include <ctime>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// Data Structures
// ----------------------------------------------------------------------------
enum class SettingType { Toggle, Slider, Enum, Color };

struct SettingValueTag { int row = -1; };
struct SettingSliderTag { int row = -1; };
struct SettingHighlight { };
struct SettingRowLabelTag { int row = -1; };

struct SettingsMenuState {
    struct Row {
        SettingType              type;
        std::string              name;
        std::string              desc;
        bool                     toggle      = false;
        float                    sliderValue = 50.f;
        float                    sliderMin   = 0.f;
        float                    sliderMax   = 100.f;
        int                      enumIndex   = 0;
        std::vector<std::string> enumValues;
        ::Color                  color       = {120,240,180,255};
    };

    std::vector<Row> rows;
    int               selected        = 0;
    float             base_y          = 140.f;
    float             row_spacing     = 40.f;
    r::ecs::Entity    highlight       = 0;
    r::ecs::Entity    description_text= 0; // (description currently static / first row only)
};

static r::ecs::Entity spawn_text(
    r::ecs::Commands &cmd,
    const std::string &txt,
    int size,
    float x,
    float y,
    int z = 10,
    r::UiTextColor col = {200,200,200,255})
{
    return cmd.spawn(
        r::UiText{ txt, (::u32)size },
        col,
        r::UiPosition{ { x, y } },
        r::UiRectSize{ { 0.f, 0.f } },
        r::UiAutoSizeText{},
        r::UiZIndex{ z },
        r::UiScale{ 1.f, 1.f }
    ).id();
}

void spawn_settings_menu_system(
    r::ecs::Commands &commands,
    r::ecs::Res<r::WindowPluginConfig> win,
    r::ecs::Res<SettingsMenuState> state,
    r::ecs::Res<r::UiTheme> theme)
{
    auto *st = const_cast<SettingsMenuState*>(state.ptr);
    auto *th = const_cast<r::UiTheme*>(theme.ptr);
    th->hover_dark_percent = 0.55f;
    th->flash_percent      = 1.15f;
    th->hover_speed        = 16.f;

    st->rows = {
        { SettingType::Toggle, "Show HUD",            "Show or hide HUD",                    true },
        { SettingType::Toggle, "Show HUD Prompts",    "Toggle contextual prompts",           false },
        { SettingType::Toggle, "Control Hints",       "Show basic control hints",            true },
        { SettingType::Toggle, "Reactive Hints",      "Enable reactive hint overlays",       false },
        { SettingType::Toggle, "HUD Motion",          "Enable subtle HUD motion",            false },
        { SettingType::Slider, "Camera Shake Amount", "Strength of camera shake",            false, 50.f, 0.f, 100.f },
        { SettingType::Enum,   "Soldier Compass",     "Compass display mode",                false, 0,0,0,0,{"ALWAYS ON","OFF"} },
        { SettingType::Enum,   "Fire Mode Indicator", "When to show fire mode",              false, 0,0,0,0,{"WHEN AVAILABLE","ALWAYS","OFF"} },
        { SettingType::Toggle, "Show Vehicle Seat",   "Show seat index in vehicles",         true },
        { SettingType::Enum,   "Colorblind",          "Colorblind filter",                   false, 0,0,0,0,{"OFF","DEUT","PROT","TRIT"} }
    };

    float W = (float)win.ptr->size.width;
    float H = (float)win.ptr->size.height;
    (void)H;

    // Background (black full-screen)
    commands.spawn(
        r::UiRectSize{ { W, H } },
        r::UiPosition{ { 0.f, 0.f } },
        r::UiColor{ 0,0,0,255 },
        r::UiOriginalColor{ 0,0,0,255 },
        r::UiStyle{},
        r::UiDirty{},
        r::UiZIndex{ 0 },
        r::UiScale{ 1.f,1.f }
    );

    // Animated title (timeline pulsing scale)
    {
        r::UiTimeline timeline;
        timeline.playing = true;
        timeline.time    = 0.f;
        timeline.tracks.push_back(
            r::UiTimelineTrack{ r::UiTimelineProperty::Scale, { {0.f,1.f},{1.f,1.05f},{2.f,1.f} }, true }
        );

        commands.spawn(
            r::UiText{ "HUD GENERAL", 48 },
            r::UiTextColor{ 180,255,255,255 },
            r::UiPosition{ { 40.f, 40.f } },
            r::UiRectSize{ { 0,0 } },
            r::UiAutoSizeText{},
            r::UiScale{ 1.f,1.f },
            timeline,
            r::UiZIndex{ 30 }
        );
    }

    // Category bar
    const char* categories[] = {
        "OPTIONS","GENERAL","DISPLAY","SOUND",
        "CONTROLLER","MOUSE & KEYBOARD","ACCESSIBILITY","EXTRAS"
    };
    float cat_x = 40.f;
    for (int i = 0; i < 8; ++i) {
        r::UiTextColor color = (i == 2)
            ? r::UiTextColor{ 120,255,255,255 }
            : r::UiTextColor{ 120,170,180,255 };
        spawn_text(commands, categories[i], 20, cat_x, 100.f, 18, color);
        cat_x += (float)(MeasureText(categories[i], 20) + 28);
    }

    // Rows (labels + values / widgets)
    float label_x = 60.f;
    float value_x = 500.f;
    float row_y   = st->base_y;
    for (int i = 0; i < (int)st->rows.size(); ++i) {
        auto &row = st->rows[i];
        commands.spawn(
            r::UiText{ row.name, 20 },
            r::UiTextColor{ 200,255,255,255 },
            r::UiPosition{ { label_x, row_y } },
            r::UiRectSize{ { 0,0 } },
            r::UiAutoSizeText{},
            r::UiScale{ 1.f,1.f },
            SettingRowLabelTag{ i }
        );

        // Precompute textual value representation
        std::string vtext;
        if (row.type == SettingType::Toggle)      vtext = row.toggle ? "ON" : "OFF";
        else if (row.type == SettingType::Slider) vtext = std::to_string((int)row.sliderValue);
        else if (row.type == SettingType::Enum)   vtext = row.enumValues[row.enumIndex];

        if (row.type == SettingType::Slider) {
            commands.spawn(
                r::UiSlider{ row.sliderValue, row.sliderMin, row.sliderMax, false },
                r::UiRectSize{ { 220.f, 14.f } },
                r::UiPosition{ { value_x, row_y + 4.f } },
                r::UiInteraction{},
                r::UiButton{},
                r::UiScale{ 1.f,1.f },
                SettingSliderTag{ i }
            );
        }

        if (row.type == SettingType::Color) {
            commands.spawn(
                r::UiRectSize{ { 120.f,22.f } },
                r::UiPosition{ { value_x, row_y + 2.f } },
                r::UiColor{ row.color.r,row.color.g,row.color.b,row.color.a },
                r::UiOriginalColor{ row.color.r,row.color.g,row.color.b,row.color.a },
                r::UiStyle{},
                r::UiDirty{},
                r::UiScale{ 1.f,1.f },
                SettingValueTag{ i }
            );
        } else {
            commands.spawn(
                r::UiText{ vtext, 20 },
                r::UiTextColor{ 0,0,0,255 },
                r::UiPosition{ { value_x, row_y } },
                r::UiRectSize{ { 0,0 } },
                r::UiAutoSizeText{},
                r::UiScale{ 1.f,1.f },
                SettingValueTag{ i }
            );
        }
        row_y += st->row_spacing;
    }

    // Highlight bar entity
    st->highlight = commands.spawn(
        r::UiRectSize{ { W - 120.f, st->row_spacing - 6.f } },
        r::UiPosition{ { 40.f, st->base_y - 3.f } },
        r::UiColor{ 0,255,255,40 },
        r::UiOriginalColor{ 0,255,255,40 },
        r::UiStyle{},
        r::UiDirty{},
        r::UiZIndex{ 5 },
        r::UiScale{ 1.f,1.f },
        SettingHighlight{}
    ).id();

    // Description (currently only first row's desc shown)
    st->description_text = spawn_text(
        commands,
        st->rows[0].desc,
        20,
        value_x + 360.f,
        st->base_y,
        18,
        r::UiTextColor{ 180,255,255,255 }
    );

    // Footer actions (dummy / visual only)
    spawn_text(commands, "BACK",            18,  60.f, (float)win.ptr->size.height - 40.f, 18, r::UiTextColor{ 150,200,200,255 });
    spawn_text(commands, "RESTORE DEFAULT", 18, 200.f, (float)win.ptr->size.height - 40.f, 18, r::UiTextColor{ 150,200,200,255 });
}

// Navigation (Up/Down)
void settings_navigation_system(r::ecs::Res<SettingsMenuState> state)
{
    auto *st = const_cast<SettingsMenuState*>(state.ptr);
    if (IsKeyPressed(KEY_DOWN)) st->selected = (st->selected + 1) % (int)st->rows.size();
    if (IsKeyPressed(KEY_UP))   st->selected = (st->selected - 1 + (int)st->rows.size()) % (int)st->rows.size();
}

// Apply value changes via Left/Right/Enter
void settings_apply_actions_system(r::ecs::Res<SettingsMenuState> state)
{
    auto *st  = const_cast<SettingsMenuState*>(state.ptr);
    auto &row = st->rows[st->selected];
    bool L = IsKeyPressed(KEY_LEFT);
    bool R = IsKeyPressed(KEY_RIGHT);
    bool A = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);

    switch (row.type) {
        case SettingType::Toggle:
            if (A || L || R) row.toggle = !row.toggle;
            break;
        case SettingType::Slider:
            if (L) row.sliderValue = std::max(row.sliderMin, row.sliderValue - 5.f);
            if (R) row.sliderValue = std::min(row.sliderMax, row.sliderValue + 5.f);
            break;
        case SettingType::Enum:
            if (L) row.enumIndex = (row.enumIndex - 1 + (int)row.enumValues.size()) % (int)row.enumValues.size();
            if (R) row.enumIndex = (row.enumIndex + 1) % (int)row.enumValues.size();
            break;
        case SettingType::Color:
            if (A) {
                static ::Color presets[] = {
                    {120,240,180,255}, {160,200,255,255}, {255,180,120,255}, {255,255,120,255}
                };
                static int idx = 0;
                idx      = (idx + 1) % 4;
                row.color = presets[idx];
            }
            break;
    }
}

// Move highlight bar to currently selected row
void settings_highlight_update_system(
    r::ecs::Query<r::ecs::Mut<r::UiPosition>, r::ecs::With<SettingHighlight>> q,
    r::ecs::Res<SettingsMenuState> state)
{
    auto *st = const_cast<SettingsMenuState*>(state.ptr);
    float y  = st->base_y + st->selected * st->row_spacing - 3.f;
    for (auto [pos, _] : q) {
        pos.ptr->pos.y = y;
    }
}

// Synchronize UI widget visuals from SettingsMenuState (one-way binding)
void settings_value_sync_system(
    r::ecs::Query<r::ecs::Mut<r::UiText>, r::ecs::Ref<SettingValueTag>> q_txt,
    r::ecs::Query<r::ecs::Mut<r::UiRectSize>, r::ecs::Mut<r::UiColor>, r::ecs::Ref<SettingValueTag>> q_color,
    r::ecs::Query<r::ecs::Mut<r::UiSlider>, r::ecs::Ref<SettingSliderTag>> q_slider,
    r::ecs::Res<SettingsMenuState> state)
{
    auto *st = const_cast<SettingsMenuState*>(state.ptr);

    // Slider numeric values
    for (auto [slider, tag] : q_slider) {
        auto &row = st->rows[tag.ptr->row];
        if (row.type == SettingType::Slider) {
            slider.ptr->value = row.sliderValue;
        }
    }

    // Color boxes
    for (auto [rect, col, tag] : q_color) {
        auto &row = st->rows[tag.ptr->row];
        if (row.type == SettingType::Color) {
            col.ptr->r = row.color.r;
            col.ptr->g = row.color.g;
            col.ptr->b = row.color.b;
            col.ptr->a = row.color.a;
            rect.ptr->size = { 120.f, 22.f };
        }
    }

    // Plain textual values
    for (auto [txt, tag] : q_txt) {
        auto &row = st->rows[tag.ptr->row];
        switch (row.type) {
            case SettingType::Toggle: txt.ptr->value = row.toggle ? "ON" : "OFF"; break;
            case SettingType::Slider: txt.ptr->value = std::to_string((int)row.sliderValue); break;
            case SettingType::Enum:   txt.ptr->value = row.enumValues[row.enumIndex]; break;
            default: break; // Color rows have no textual overlay here
        }
    }
}

// Overlay debug (very lightweight info header)
void settings_overlay_render(r::ecs::Res<SettingsMenuState> state)
{
    auto *st = const_cast<SettingsMenuState*>(state.ptr);
    DrawText(
        TextFormat("Selected: %s", st->rows[st->selected].name.c_str()),
        40, 10, 20,
        Color{ 180,255,255,255 }
    );
    DrawFPS(1180, 10);
}

int main()
{
    srand((unsigned)time(nullptr));

    r::Application{}
        .add_plugins(
            r::DefaultPlugins{}.set(
                r::WindowPlugin{ r::WindowPluginConfig{ .size = {1280,720}, .title = "Settings Menu Demo" }}
            )
        )
        .insert_resource(SettingsMenuState{})
        .add_systems(r::Schedule::STARTUP, spawn_settings_menu_system)
        .add_systems(
            r::Schedule::UPDATE,
            settings_navigation_system,
            settings_apply_actions_system,
            settings_highlight_update_system,
            settings_value_sync_system
        )
        .add_systems(r::Schedule::RENDER, settings_overlay_render)
        .run();

    return 0;
}

/* End Settings Menu Demo */
