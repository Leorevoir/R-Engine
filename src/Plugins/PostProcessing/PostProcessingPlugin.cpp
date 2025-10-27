#include <R-Engine/Plugins/PostProcessingPlugin.hpp>
#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

#include <R-Engine/Application.hpp>

#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Core/Logger.hpp>

namespace {

/**
 * globals
 */

static struct {
        const std::string name;
        ::Shader shader;
} g_shaders[] = {
    {.name = r::path::get("assets/shaders/postprocessing/grayscale.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/posterization.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/dream_vision.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/pixelizer.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/cross_hatching.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/cross_stitching.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/predator.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/scanlines.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/fisheye.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/sobel.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/bloom.frag"), .shader = {}},
    {.name = r::path::get("assets/shaders/postprocessing/blur.frag"), .shader = {}},
};

static struct {
        bool initialized = false;
        ::RenderTexture2D target;
} g_render_texture;

/**
 * static helpers
 */

static inline void backend_begin_texture_mode(void) noexcept
{
    EndTextureMode();
}

static inline void backend_end_texture_mode(void) noexcept
{
    EndShaderMode();
}

static inline void backend_draw_texture_rec(void) noexcept
{
    const auto &texture = g_render_texture.target.texture;

    DrawTextureRec(texture, {0, 0, static_cast<f32>(texture.width), static_cast<f32>(-texture.height)}, {0, 0}, WHITE);
}

/**
 * systems
 */

static void post_processing_plugin_startup(const r::ecs::Res<r::WindowPluginConfig> config_ptr) noexcept
{
    const r::Vec2i w_size = static_cast<r::Vec2i>(config_ptr.ptr->size);

    g_render_texture.target = LoadRenderTexture(w_size.x, w_size.y);
    g_render_texture.initialized = true;

    for (auto &fx : g_shaders) {
        fx.shader = LoadShader(nullptr, fx.name.c_str());
    }
}

static void post_processing_plugin_begin_capture(r::ecs::Res<r::RenderPluginConfig> config) noexcept
{
    if (g_render_texture.initialized) {
        BeginTextureMode(g_render_texture.target);

        const ::Color rl_color = {
            .r = config.ptr->clear_color.r,
            .g = config.ptr->clear_color.g,
            .b = config.ptr->clear_color.b,
            .a = config.ptr->clear_color.a,
        };
        ClearBackground(rl_color);
    }
}

static void post_processing_plugin_end_capture_and_draw(const r::ecs::Res<r::PostProcessingPluginConfig> config_ptr) noexcept
{
    if (!g_render_texture.initialized) {
        return;
    }

    backend_begin_texture_mode();

    const auto &state = config_ptr.ptr->state;

    if (state == r::PostProcessingState::Disabled) {
        backend_draw_texture_rec();
        return;
    }

    BeginShaderMode(g_shaders[static_cast<u32>(state)].shader);
    backend_draw_texture_rec();
    backend_end_texture_mode();
}

static void post_processing_plugin_shutdown(void) noexcept
{
    for (auto &fx : g_shaders) {
        UnloadShader(fx.shader);
    }
    if (g_render_texture.initialized) {
        UnloadRenderTexture(g_render_texture.target);
        g_render_texture.initialized = false;
    }
}

}// namespace

r::PostProcessingPlugin::PostProcessingPlugin(PostProcessingPluginConfig config) noexcept : _config(std::move(config))
{
    /* __ctor__ */
}

void r::PostProcessingPlugin::build(Application &app)
{
    app.insert_resource(_config);

    app.add_systems<post_processing_plugin_startup>(Schedule::STARTUP);
    app.add_systems<post_processing_plugin_begin_capture>(Schedule::BEFORE_RENDER_3D);
    app.add_systems<post_processing_plugin_end_capture_and_draw>(Schedule::RENDER_2D).after<r::ui::render_system>();
    app.add_systems<post_processing_plugin_shutdown>(Schedule::SHUTDOWN);
}
